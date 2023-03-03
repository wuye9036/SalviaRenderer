from Meta import Sender, Receiver, Args, pipeable
from Utils import trace_func


class ThenSender(Sender):
  def __init__(self, pred, fn):
    self._pred = pred
    self._fn = fn

  @trace_func
  def connect(self, receiver):
    return self._pred.connect(
      ThenReceiver(self._fn, receiver)
    )


class ThenReceiver(Receiver):
  def __init__(self, fn, receiver: Receiver):
    self._fn = fn
    self._receiver = receiver

  @trace_func
  def set_value(self, *args, **kwargs):
    result = self._fn(*args, **kwargs)
    if isinstance(result, Args):
      result.invoke(self._receiver.set_value)
    else:
      self._receiver.set_value(result)

  @trace_func
  def set_done(self):
    self._receiver.set_done()

  @trace_func
  def get_scheduler(self):
    return self._receiver.get_scheduler()


@pipeable
def then(sender, fn):
  return ThenSender(sender, fn)
