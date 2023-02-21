from Utils import trace_test, trace_func
from TrampolineScheduler import trampoline_scheduler
from SingleThreadContext import single_thread_context, manual_event_loop
from Meta import chain
from Meta import Args
from Then import then


class let_value_op:
  def __init__(self, pred, succFact, receiver) -> None:
    self.succFact = succFact
    self.receiver = receiver
    self._predOp = pred.connect(let_value_pred_receiver(self))
    self._values = Args()

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
    self._op.receiver.set_value(*args, **kwargs)


class let_value_pred_receiver:
  def __init__(self, op):
    self._op = op

  @trace_func
  def set_value(self, *args, **kw_args):
    succ_factory_args = Args(*args, **kw_args)
    succ_operation = succ_factory_args.invoke(self._op.succFact).connect(let_value_succ_receiver(self._op))
    succ_operation.start()


def let_value(pred, succFact):
  return let_value_pred_sender(pred, succFact)


class sync_wait_receiver:
  def __init__(self, ctx):
    self._ctx = ctx

  @trace_func
  def set_value(self, *_args, **_kwargs):
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
    return Args()

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
