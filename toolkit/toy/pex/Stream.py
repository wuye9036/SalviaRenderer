import abc
import enum
import typing
from Meta import continuation_style, Pipeable, Sender, Receiver, OperationState, Unit
from Then import then
from Utils import trace_func


class BlockingKind(enum.Enum):
  Maybe = "Maybe"
  Never = "Never"
  Always = "Always"
  AlwaysInline = "AlwaysInline"


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

  def next(self):
    return NextSender(self)

  def cleanup(self):
    return ReadyDoneSender()


def range_stream(beg, end):
  # Similar to C++ cases
  return RangeStream(beg, end)


class RangeStreamOperation:
  def __init__(self, stream: RangeStream, receiver: Receiver):
    self._stream = stream
    self._receiver = receiver

  def start(self):
    if self._stream.iter_value < self._stream.end_value:
      v = self._stream.iter_value
      self._stream.iter_value += 1
      self._receiver.set_value(v)
    else:
      self._receiver.set_done()


class NextSender(Sender):
  def __init__(self, stream):
    self._stream = stream

  @trace_func
  def connect(self, receiver: Receiver):
    return RangeStreamOperation(self._stream, receiver)

  @staticmethod
  def blocking():
    return BlockingKind.AlwaysInline


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
class NextStreamDecorator(Stream):
  def __init__(self,
               stream: Stream,
               decoration_func: typing.Callable[[Sender], Sender]):
    self._decoration_func = decoration_func
    self._stream = stream

  @trace_func
  def next(self) -> Sender:
    next_sender = self._stream.next()
    return self._decoration_func(next_sender)

  @trace_func
  def cleanup(self):
    return self._stream.cleanup()


def decorate_next_stream(stream, fn):
  # fn: Callable[Sender] -> Sender
  return NextStreamDecorator(stream, fn)


@continuation_style
def transform_stream(stream, fn):
  def _transform(sender):
    return then(sender, fn)
  return NextStreamDecorator(stream, _transform)


def make_for_each_map(map_fn):
  def _impl(_s: Unit, *args, **kwargs):
    map_fn(*args, **kwargs)
  return _impl


@trace_func
def for_each_reduce(_init_value: Unit):
  pass


class ReduceOperation(OperationState):
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
      NextReceiver(self)
    )
    self.nextOp.start()


class DoneCleanupReceiver(Receiver):
  def __init__(self, op: ReduceOperation):
    self._op = op

  @trace_func
  def set_done(self):
    op = self._op
    op.receiver.set_value(op.state)

  @trace_func
  def set_value(self):
    raise RuntimeError("This function is not expected to be invoked")


class NextReceiver(Receiver):
  def __init__(self, op):
    assert isinstance(op, ReduceOperation)
    self._op = op

  @trace_func
  def set_value(self, *args, **kwargs):
    op = self._op
    op.state = op.reducer(op.state, *args, **kwargs)
    op.next = op.stream.next().connect(NextReceiver(self._op))
    op.next.start()

  @trace_func
  def set_done(self):
    op = self._op
    op.doneOp = op.stream.cleanup().connect(DoneCleanupReceiver(op))
    op.doneOp.start()


class ReduceStreamSender(Sender):
  def __init__(self, stream: Stream, init_value, reducer):
    self._stream = stream
    self._init_value = init_value
    self._reducer = reducer

  @trace_func
  def connect(self, receiver: Receiver):
    return ReduceOperation(
      self._stream, self._init_value, self._reducer, receiver)


def reduce_stream(stream: Stream, init_value, reducer):
  return ReduceStreamSender(stream, init_value, reducer)


@continuation_style
def for_each(stream: Stream, fn):
  reduced_stream = reduce_stream(stream, Unit(), make_for_each_map(fn))
  return then(reduced_stream, for_each_reduce)
