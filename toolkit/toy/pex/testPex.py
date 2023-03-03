import unittest

from Utils import trace_test, trace_func
from Utils import config as trace_config
from TrampolineScheduler import trampoline_scheduler
from SingleThreadContext import single_thread_context
from Meta import chain, Args, Sender
from Then import then
from SenderConsumers import sync_wait
from Stream import Stream, range_stream, transform_stream, for_each, via_stream
from Schedule import schedule
from WithQueryValue import with_query_value


# ... TO REMAKE ...
# Algorithms:
#   let_value
#   just
#   transfer
#   bulk
#   repeat_effect
#   repeat_effect_until
# Contexts and schedulers:
#   timed_single_thread_context
#   schedule_at
#   schedule_after
# ... ... ... ... ...


def inc_count(count, addend):
  @trace_func(postfix=f"{addend}")
  def _inc_impl():
    count[0] += addend
    return Args()

  return _inc_impl


class PexTest(unittest.TestCase):
  def setUp(self) -> None:
    trace_config.get().trace_func_enabled = False

  @trace_test
  def testThen(self):
    with single_thread_context() as context:
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

      self.assertEqual(count[0], 3)

  @trace_test
  def testThenChain(self):
    with single_thread_context() as context:
      scheduler = context.get_scheduler()

      count = [0]

      sync_wait(
        chain(
          scheduler.schedule(),
          (then, inc_count(count, 1)),
          (then, inc_count(count, 2))
        )
      )

      self.assertEqual(count[0], 3)

  @trace_test
  def testThenPipe(self):
    with single_thread_context() as context:
      scheduler = context.get_scheduler()

      count = [0]
      sync_wait(
        scheduler.schedule() | then(inc_count(count, 1)) | then(inc_count(count, 2))
      )

      self.assertEqual(count[0], 3)

  @trace_test
  def testTrampolineScheduler(self):
      scheduler = trampoline_scheduler(1)

      count = [0]
      sync_wait(
        scheduler.schedule() | then(inc_count(count, 1)) | then(inc_count(count, 2))
      )

      self.assertEqual(count[0], 3)

  @trace_test
  def testStreamPipe(self):
    n_sum = [0]

    def _accumulate(v):
      n_sum[0] += v
      return v

    s = range_stream(0, 10) | transform_stream(lambda v: v * v)
    self.assertIsInstance(s, Stream)
    s = s | for_each(_accumulate)
    self.assertIsInstance(s, Sender)

    sync_wait(s)

    self.assertEqual(n_sum[0], sum(i*i for i in range(10)))

  @trace_test
  def testForEachViaTrampoline(self):
    sync_wait(
      range_stream(0, 10)
      | transform_stream(lambda v: v*v)
      | via_stream(trampoline_scheduler(1))
      | for_each(lambda v: print(f"for_each accepted value: {v}."))
      | then(lambda: print("for_each done."))
    )
    pass

  @trace_test
  def testGetScheduler(self):
    # Test default schedule on sync_wait.receiver
    sync_wait(schedule() | then(lambda: 0))

    with single_thread_context() as ctx:
      sync_wait(with_query_value(
        schedule(),
        'get_scheduler',
        ctx.get_scheduler()
      ))


if __name__ == "__main__":
  unittest.main()
