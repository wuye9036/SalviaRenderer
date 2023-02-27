from Utils import trace_func
from Meta import Receiver
from SingleThreadContext import manual_event_loop


class SyncWaitReceiver(Receiver):
  def __init__(self, ctx):
    self._ctx = ctx

  @trace_func
  def set_value(self, *_args, **_kwargs):
    self.signal_complete()

  def set_done(self):
    self.signal_complete()

  def signal_complete(self):
    self._ctx.stop()


@trace_func
def sync_wait(sender):
  ctx = manual_event_loop()
  op = sender.connect(SyncWaitReceiver(ctx))
  op.start()
  ctx.run()
