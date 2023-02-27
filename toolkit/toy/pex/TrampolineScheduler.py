import threading
import typing
from Utils import trace_func
from Meta import Sender


class TrampolineScheduler:
  def __init__(self, maxRecursionDepth: int):
    self._maxRecursionDepth = maxRecursionDepth

  def schedule(self):
    return TrampolineSender(self._maxRecursionDepth)


def trampoline_scheduler(maxRecursionDepth: int):
  return TrampolineScheduler(maxRecursionDepth)


class TrampolineSchedulerOperation:
  def __init__(self, receiver, maxRecursionDepth):
    self.next = None
    self._maxRecursionDepth = maxRecursionDepth
    self._receiver = receiver

  @trace_func
  def execute(self):
    self._receiver.set_value()

  @trace_func
  def start(self):
    current_state: typing.Optional[TrampolineState] = TrampolineState.current()
    if current_state is None:
      with TrampolineState.create() as state:
        self.execute()
        state.drain()
    elif current_state.recursionDepth < self._maxRecursionDepth:
      current_state.recursionDepth += 1
      self.execute()
    else:
      self.next = current_state.head
      current_state.head = self


class TrampolineState:
  _current = threading.local()

  def __init__(self):
    self.recursionDepth = 1
    self.head = None

  def __enter__(self):
    TrampolineState._current.state = self
    return self

  def __exit__(self, exc_type, exc_val, exc_tb):
    TrampolineState._current.state = None

  @staticmethod
  def create():
    return TrampolineState()

  @staticmethod
  def current():
    try:
      state: TrampolineState = TrampolineState._current.state
      return state
    except AttributeError:
      return None

  @trace_func
  def drain(self):
    while self.head is not None:
      op: TrampolineSchedulerOperation = self.head
      self.head = op.next
      self.recursionDepth = 1
      op.execute()


class TrampolineSender(Sender):
  def __init__(self, maxDepth: int):
    self._maxRecursionDepth = maxDepth

  @trace_func
  def connect(self, receiver):
    return TrampolineSchedulerOperation(receiver, self._maxRecursionDepth)
