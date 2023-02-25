import abc
import enum
from Meta import continuation_style, Pipeable, Sender
from Then import then


class BlockingKind:
  Maybe = "Maybe"
  Never = "Never"
  Always = "Always"
  AlwaysInline = "AlwaysInline"


class Stream(Pipeable):
  @abc.abstractmethod
  def next(self):
    pass

  @abc.abstractmethod
  def clean(self):
    pass


class operation:
  def __init__(self, stream, receiver):
    self._stream = stream
    self._receiver = receiver

  def start(self):
    if self._stream.next < self._stream.end:
      v = self._stream.next
      self._stream.next += 1
      self._receiver.set_value(v)


class next_sender(Sender):
  def __init__(self, stream):
    self._stream = stream

  def connect(self, receiver):
    return operation(self._stream, receiver)

  @staticmethod
  def blocking(self):
    return BlockingKind.AlwaysInline


class range_stream(Stream):
  def __init__(self, beg, end):
    self.next = beg
    self.end = end

  def next(self):
    return next_sender(self)

  def clean(self):
    raise NotImplementedError()
    # return ready_done_sender()


# eqv to libunifex::stream_adaptor which is used for chaining functions for stream processing.
class next_stream_decorator(Stream):
  def __init__(self, stream, decoration_func):
    assert isinstance(stream, Stream)
    self._decoration_func = decoration_func
    self._stream = stream

  def next(self):
    return self._decoration_func(self._stream.next())

  def clean(self):
    return self._stream.clean()


def next_decorate_stream(stream, fn):
  """
  fn: Callable[Sender] -> Sender
  """
  return next_stream_decorator(stream, fn)


@continuation_style
def transform_stream(stream, fn):
  def _transform(sender):
    return then(sender, fn)
  return next_stream_decorator(stream, _transform)


