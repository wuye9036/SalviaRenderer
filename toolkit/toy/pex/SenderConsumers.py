from Utils import trace_func
from Meta import Receiver
from SingleThreadContext import EventLoop


class SyncWaitReceiver(Receiver):
  def __init__(self, ctx):
    self._ctx = ctx

  @trace_func
  def set_value(self, *_args, **_kwargs):
    self.signal_complete()

  @trace_func
  def set_done(self):
    self.signal_complete()

  def signal_complete(self):
    self._ctx.stop()


@trace_func
def sync_wait(sender):
  ctx = EventLoop()
  op = sender.connect(SyncWaitReceiver(ctx))
  op.start()
  ctx.run()
