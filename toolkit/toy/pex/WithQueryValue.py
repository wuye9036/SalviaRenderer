from Meta import continuation_style, Sender, Receiver, OperationState


class WithQueryValueReceiver(Receiver):
  def __init__(self, receiver, value, cpo_name: str):
    self._receiver = receiver
    self._value = value
    self._cpo_name = cpo_name

  def __getattribute__(self, item):
    cpo_name = object.__getattribute__(self, "_cpo_name")
    value = object.__getattribute__(self, "_value")
    def _get_value(): return value

    if item == cpo_name:
      return _get_value

    return object.__getattribute__(self, item)

  # Just for make link shut mouse up.
  def set_value(self, *args, **kwargs):
    raise NotImplementedError()

  def set_done(self):
    raise NotImplementedError()


class WithQueryValueOperation(OperationState):
  def __init__(self, sender: Sender, receiver: Receiver, value, cpo_name: str):
    self._value = value
    self._innerOp = sender.connect(WithQueryValueReceiver(receiver, self._value, cpo_name))

  def start(self):
    self._innerOp.start()


class WithQueryValueSender(Sender):
  def __init__(self, sender: Sender, cpo_name: str, value):
    self._sender = sender
    self._value = value
    self._cpo_name = cpo_name

  def connect(self, receiver: Receiver):
    return WithQueryValueOperation(self._sender, receiver, self._value, self._cpo_name)


@continuation_style
def with_query_value(sender: Sender, cpo_name: str, value):
  return WithQueryValueSender(sender, cpo_name, value)