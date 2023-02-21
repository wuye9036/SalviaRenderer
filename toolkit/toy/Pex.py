import threading
import typing
from PexUtils import trace_test, trace_func
from PexTrampoline import trampoline_scheduler
from PexFunc import algo_as_continuation, Sender


class va_pack:
  def __init__(self, *args, **kwargs) -> None:
    self._args = args
    self._kwargs = kwargs

  def invoke(self, fn, *prior_args):
    return fn(*prior_args, *self._args, **self._kwargs)


class task_base:
  def __init__(self, executeFn):
    self._next = None
    self._execute = executeFn

  def execute(self):
    self._execute(self)


class loop_sched_operation(task_base):
  def __init__(self, receiver, loop):
    self._receiver = receiver
    self._loop = loop
    super().__init__(loop_sched_operation._execute_impl)

  @trace_func
  def start(self):
    self._loop.enqueue(self)

  @staticmethod
  def _execute_impl(t):
    assert isinstance(t, loop_sched_operation)
    _self = t
    _self._receiver.set_value()


class loop_scheduler_task:
  def __init__(self, loop):
    self._loop = loop

  @trace_func
  def connect(self, receiver):
    return loop_sched_operation(receiver, self._loop)


class loop_scheduler:
  def __init__(self, loop) -> None:
    self._loop = loop

  def schedule(self):
    return Sender(loop_scheduler_task(self._loop))


class manual_event_loop:  # manual_event_loop in libunifex
  def __init__(self) -> None:
    self._head: typing.Optional[task_base] = None
    self._tail: typing.Optional[task_base] = None
    self._stop = False
    # Mutex and condition variable are ignored in this demo.

  def get_scheduler(self):
    return loop_scheduler(self)

  def run(self):
    while True:
      while self._head is None:
        if self._stop:
          return
      task = self._head
      self._head = task._next
      if self._head is None:
        self._tail = None
      task.execute()

  def stop(self):
    self._stop = True

  def enqueue(self, t):
    if self._head is None:
      self._head = t
    else:
      self._tail._next = t

    self._tail = t
    self._tail._next = None


class single_thread_context:
  def __init__(self) -> None:
    self._loop = manual_event_loop()
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

  @staticmethod
  def create():
    return single_thread_context()


class timed_single_thread_context:
  def __init__(self):
    self._head = None
    self._stop = False
    self._thread = threading.Thread(target=lambda: self.run())

  def _enqueue(self):
    raise NotImplementedError()

  def run(self):
    raise NotImplementedError()


class then_sender:
  def __init__(self, pred, fn):
    self._pred = pred
    self._fn = fn

  @trace_func
  def connect(self, receiver):
    return self._pred.connect(
      then_receiver(self._fn, receiver)
    )


class then_receiver:
  def __init__(self, fn, receiver):
    self._fn = fn
    self._receiver = receiver

  @trace_func
  def set_value(self, *args, **kwargs):
    result = self._fn(*args, **kwargs)
    assert isinstance(result, va_pack)
    result.invoke(self._receiver.set_value)


@algo_as_continuation
def then(sender, fn):
  return then_sender(sender, fn)


class let_value_op:
  def __init__(self, pred, succFact, receiver) -> None:
    self._succFact = succFact
    self._receiver = receiver
    self._predOp = pred.connect(let_value_pred_receiver(self))
    self._values = va_pack

  def start(self):
    self._predOp.start()


class let_value_pred_sender:
  def __init__(self, pred, succFact):
    self._pred = pred
    self._succFact = succFact

  @trace_func
  def connect(self, receiver):
    return let_value_op(self._pred, self._succFact, receiver)


class let_value_succ_receiver:
  def __init__(self, op):
    self._op = op

  @trace_func
  def set_value(self, *args, **kwargs):
    self._op._receiver.set_value(*args, **kwargs)


class let_value_pred_receiver:
  def __init__(self, op):
    self._op = op

  @trace_func
  def set_value(self, *args, **kw_args):
    succOp = self.va_pack.invoke(self._op._succFact).connect(let_value_succ_receiver(self._op))
    succOp.start()


def let_value(pred, succFact):
  return let_value_pred_sender(pred, succFact)


class sync_wait_receiver:
  def __init__(self, ctx):
    self._ctx = ctx

  @trace_func
  def set_value(self, *args, **kwargs):
    self.signal_complete()

  def signal_complete(self):
    self._ctx.stop()


@trace_func
def sync_wait(sender):
  ctx = manual_event_loop()
  op = sender.connect(sync_wait_receiver(ctx))
  op.start()
  ctx.run()
  print("sync_wait completed")


def chain(snd, *snd_fact_and_args):
  for fact_n_arg in snd_fact_and_args:
    snd = fact_n_arg[0](snd, *fact_n_arg[1:])
  return snd


# ... TO REMAKE ...
# Algorithms:
#   let_value
#   just
#   transfer
#   bulk
#   repeat_effect_until
# Contexts and schedulers:
#   timed_single_thread_context
#   trampoline_scheduler
#   schedule_at
#   schedule_after
# ... ... ... ... ...


def inc_count(count, addend):
  @trace_func(postfix=f"{addend}")
  def _inc_impl():
    count[0] += addend
    return va_pack()

  return _inc_impl


@trace_test
def testThen():
  with single_thread_context.create() as context:
    scheduler = context.get_scheduler()

    count = [0]

    sync_wait(
      then(
        then(
          scheduler.schedule(), inc_count(count, 1)
        ),
        inc_count(count, 2)
      )
    )

    assert count[0] == 3


@trace_test
def testThenChain():
  with single_thread_context.create() as context:
    scheduler = context.get_scheduler()

    count = [0]

    sync_wait(
      chain(
        scheduler.schedule(),
        (then, inc_count(count, 1)),
        (then, inc_count(count, 2))
      )
    )

    assert count[0] == 3


@trace_test
def testThenPipe():
  with single_thread_context.create() as context:
    scheduler = context.get_scheduler()

    count = [0]
    sync_wait(
      scheduler.schedule() | then(inc_count(count, 1)) | then(inc_count(count, 2))
    )

    assert count[0] == 3


@trace_test
def testTrampolineSched():
    scheduler = trampoline_scheduler(1)

    count = [0]
    sync_wait(
      scheduler.schedule() | then(inc_count(count, 1)) | then(inc_count(count, 2))
    )

    assert count[0] == 3


@trace_test
def testForEachViaTrampoline():
  """
  sync_wait(
    typed_via_stream(
      trampoline_scheduler(1),
      range_stream(0, 10) | transform_stream(lambda v: v*v)
    )
    | for_each(lambda v: print(v))
    | then(lambda: print("done")
  )
  """
  pass


def _main():
  testThen()
  testThenChain()
  testThenPipe()
  testTrampolineSched()
  testForEachViaTrampoline()

if __name__ == "__main__":
  _main()
