import threading
import typing
from Utils import trace_func
from Meta import Sender


class TaskBase:
  def __init__(self, executeFn):
    self.next = None
    self._execute = executeFn

  def execute(self):
    self._execute(self)


class LoopSchedulerOperation(TaskBase):
  def __init__(self, receiver, loop):
    self._receiver = receiver
    self._loop = loop
    super().__init__(LoopSchedulerOperation._execute_impl)

  @trace_func
  def start(self):
    self._loop.enqueue(self)

  @staticmethod
  def _execute_impl(t):
    assert isinstance(t, LoopSchedulerOperation)
    _self = t
    _self._receiver.set_value()


class LookSchedulerTaskAsSender(Sender):
  def __init__(self, loop):
    self._loop = loop

  @trace_func
  def connect(self, receiver):
    return LoopSchedulerOperation(receiver, self._loop)


class LoopScheduler:
  def __init__(self, loop) -> None:
    self._loop = loop

  @trace_func
  def schedule(self):
    return LookSchedulerTaskAsSender(self._loop)


class EventLoop:  # manual_event_loop in libunifex
  def __init__(self) -> None:
    self._head: typing.Optional[TaskBase] = None
    self._tail: typing.Optional[TaskBase] = None
    self._stop = False
    # Mutex and condition variable are ignored in this demo.

  def get_scheduler(self):
    return LoopScheduler(self)

  def run(self):
    while True:
      while self._head is None:
        if self._stop:
          return
      task = self._head
      self._head = task.next
      if self._head is None:
        self._tail = None
      task.execute()

  def stop(self):
    self._stop = True

  def enqueue(self, t):
    if self._head is None:
      self._head = t
    else:
      self._tail.next = t

    self._tail = t
    self._tail.next = None


class SingleThreadContext:
  def __init__(self) -> None:
    self._loop = EventLoop()
    self._thread = threading.Thread(target=lambda: self._loop.run())
    self._thread.start()
    print(f"<{self.__class__.__name__}>'s thread is launched")

  def get_scheduler(self):
    return self._loop.get_scheduler()

  def __enter__(self):
    return self

  def __exit__(self, *args):
    self._loop.stop()
    self._thread.join()


class TimedSingleThreadContext:
  def __init__(self):
    self._head = None
    self._stop = False
    self._thread = threading.Thread(target=lambda: self.run())

  def _enqueue(self):
    raise NotImplementedError()

  def run(self):
    raise NotImplementedError()


def single_thread_context():
  return SingleThreadContext()


def time_single_thread_context():
  return TimedSingleThreadContext()

