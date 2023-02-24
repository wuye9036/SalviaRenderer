from Meta import Sender, Args, continuation_style
from Utils import trace_func


class then_sender(Sender):
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
    assert isinstance(result, Args)
    result.invoke(self._receiver.set_value)


@continuation_style
def then(sender, fn):
  return then_sender(sender, fn)
