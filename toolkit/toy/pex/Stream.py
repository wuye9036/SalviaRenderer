import abc
import typing
from Meta import Args, continuation_style, Pipeable
from Meta import Sender, Receiver, OperationState, Unit, SenderDecorator
from Then import then
from Via import via
from On import on
from Utils import trace_func


class Stream(Pipeable):
  @abc.abstractmethod
  def next(self) -> Sender:
    pass

  @abc.abstractmethod
  def cleanup(self) -> Sender:
    pass


class RangeStream(Stream):
  def __init__(self, beg, end):
    self.iter_value = beg
    self.end_value = end

  @trace_func
  def next(self):
    return RangeStreamNextSender(self)

  @trace_func
  def cleanup(self):
    return ReadyDoneSender()


def range_stream(beg, end):
  # Entry function for the same form with C++ version
  return RangeStream(beg, end)


class RangeStreamOperation:
  def __init__(self, stream: RangeStream, receiver: Receiver):
    self._stream = stream
    self._receiver = receiver

  @trace_func
  def start(self):
    if self._stream.iter_value < self._stream.end_value:
      v = self._stream.iter_value
      self._stream.iter_value += 1
      self._receiver.set_value(v)
    else:
      self._receiver.set_done()


class RangeStreamNextSender(Sender):
  def __init__(self, stream):
    self._stream = stream

  @trace_func
  def connect(self, receiver: Receiver):
    return RangeStreamOperation(self._stream, receiver)


class ReadyDoneOperation(OperationState):
  def __init__(self, receiver: Receiver):
    self._receiver = receiver

  @trace_func
  def start(self):
    self._receiver.set_done()


class ReadyDoneSender(Sender):
  @trace_func
  def connect(self, receiver: Receiver) -> OperationState:
    return ReadyDoneOperation(receiver)


# eqv to libunifex::stream_adaptor which is used for chaining functions for stream processing.
class DecoratedStream(Stream):
  def __init__(self,
               stream: Stream,
               next_decorator: SenderDecorator,
               cleanup_decorator: SenderDecorator):
    self._next_decorator = next_decorator
    self._cleanup_decorator = cleanup_decorator
    self._stream = stream

  @trace_func
  def next(self) -> Sender:
    next_sender = self._stream.next()
    ret = self._next_decorator(next_sender)
    assert isinstance(ret, Sender)
    return ret

  @trace_func
  def cleanup(self):
    cleanup_sender = self._stream.cleanup()
    ret = self._cleanup_decorator(cleanup_sender)
    assert isinstance(ret, Sender)
    return ret


def decorate_stream(stream: Stream,
                    next_decorator: SenderDecorator,
                    cleanup_decorator: SenderDecorator):
  return DecoratedStream(stream, next_decorator, cleanup_decorator)


@continuation_style
def transform_stream(stream, fn):
  def _transform(sender):
    return then(sender, fn)
  return decorate_stream(stream, _transform, lambda s: s)


class ReduceStreamOperation(OperationState):
  def __init__(self, stream: Stream, state, reducer, receiver: Receiver):
    self.stream = stream
    self.state = state
    self.reducer = reducer
    self.receiver = receiver

    self.nextOp: typing.Optional[OperationState] = None
    self.doneOp: typing.Optional[OperationState] = None

  @trace_func
  def start(self):
    self.nextOp = self.stream.next().connect(
      ReduceStreamNextReceiver(self)
    )
    self.nextOp.start()


class ReduceStreamDoneCleanupReceiver(Receiver):
  def __init__(self, op: ReduceStreamOperation):
    self._op = op

  @trace_func
  def set_done(self):
    op = self._op
    op.receiver.set_value(op.state)

  @trace_func
  def set_value(self):
    raise RuntimeError("This function is not expected to be invoked")


class ReduceStreamNextReceiver(Receiver):
  def __init__(self, op):
    assert isinstance(op, ReduceStreamOperation)
    self._op = op

  @trace_func
  def set_value(self, *args, **kwargs):
    op = self._op
    op.state = op.reducer(op.state, *args, **kwargs)
    op.next = op.stream.next().connect(ReduceStreamNextReceiver(self._op))
    op.next.start()

  @trace_func
  def set_done(self):
    op = self._op
    op.doneOp = op.stream.cleanup().connect(ReduceStreamDoneCleanupReceiver(op))
    op.doneOp.start()


class ReduceStreamSender(Sender):
  def __init__(self, stream: Stream, init_value, reducer):
    self._stream = stream
    self._init_value = init_value
    self._reducer = reducer

  @trace_func
  def connect(self, receiver: Receiver):
    return ReduceStreamOperation(
      self._stream, self._init_value, self._reducer, receiver)


def reduce_stream(stream: Stream, init_value, reducer):
  return ReduceStreamSender(stream, init_value, reducer)


@continuation_style
def for_each(stream: Stream, fn):
  def _map(_s: Unit, *args, **kwargs): return fn(*args, **kwargs)

  def _dummy_reduce(_s: Unit): return Args()

  reduced_stream = reduce_stream(stream, Unit(), _map)
  return then(reduced_stream, _dummy_reduce)


@continuation_style
def via_stream(stream: Stream, scheduler):
  def _reschedule(sender: Sender):
    return via(scheduler, sender)
  return decorate_stream(stream, _reschedule, _reschedule)


def on_stream(stream: Stream, scheduler):
  def _reschedule(sender: Sender):
    return on(scheduler, sender)
  return decorate_stream(stream, _reschedule, _reschedule)
