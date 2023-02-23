class operation:
  def __init__(self, stream, receiver):
    self._stream = stream
    self._receiver = receiver

  def start(self):
    if self._stream.next < self._stream.end:
      v = self._stream.next
      self._stream.next += 1
      self._receiver.set_value(v)


class next_sender:
  def __init__(self, stream):
    self._stream = stream

  def connect(self, receiver):
    return operation(self._stream, receiver)


class range_stream:
  def __init__(self, beg, end):
    self.next = beg
    self.end = end

  def next(self):
    return next_sender(self)

  def clean(self):
    raise NotImplementedError()
    # return ready_done_sender()

