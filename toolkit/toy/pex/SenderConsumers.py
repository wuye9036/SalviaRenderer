from Utils import trace_func
from SingleThreadContext import manual_event_loop


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
