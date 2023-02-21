import threading
import typing
from PexUtils import trace_func
from PexFunc import Sender


class trampoline_scheduler:
  def __init__(self, maxRecursionDepth: int):
    self._maxRecursionDepth = maxRecursionDepth

  def schedule(self):
    return Sender(sender(self._maxRecursionDepth))


class operation:
  def __init__(self, receiver, maxRecursionDepth):
    self.next = None
    self._maxRecursionDepth = maxRecursionDepth
    self._receiver = receiver

  def execute(self):
    self._receiver.set_value()

  @trace_func
  def start(self):
    current_state: typing.Optional[trampoline_state] = trampoline_state.current()
    if current_state is None:
      with trampoline_state.create() as state:
        self.execute()
        state.drain()
    elif current_state.recursionDepth < self._maxRecursionDepth:
      current_state.recursionDepth += 1
      self.execute()
    else:
      self.next = current_state.head
      current_state.head = self


class trampoline_state:
  _current = threading.local()

  def __init__(self):
    self.recursionDepth = 1
    self.head = None

  def __enter__(self):
    trampoline_state._current.state = self
    return self

  def __exit__(self, exc_type, exc_val, exc_tb):
    trampoline_state._current.state = None

  @staticmethod
  def create():
    return trampoline_state()

  @staticmethod
  def current():
    try:
      state: trampoline_state = trampoline_state._current.state
      return state
    except AttributeError:
      return None

  def drain(self):
    while self.head is not None:
      op: operation = self.head
      self.head = op.next
      self.recursionDepth = 1
      op.execute()


class sender:
  def __init__(self, maxDepth: int):
    self._maxRecursionDepth = maxDepth

  @trace_func
  def connect(self, receiver):
    return operation(receiver, self._maxRecursionDepth)
