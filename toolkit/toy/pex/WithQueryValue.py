from Meta import *


class WithQueryValueReceiver(Receiver):
  def __init__(self, receiver, value, cpo_name: str):
    self._receiver = receiver
    self._value = value

    self.customization_table = {
      cpo_name: lambda: self._value
    }

  @customizable
  def set_value(self, *args, **kwargs):
    self._receiver.set_value(*args, **kwargs)

  @customizable
  def set_done(self):
    self._receiver.set_done()

  @customizable
  def get_scheduler(self) -> Scheduler:
    return self._receiver.get_scheduler()


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


@pipeable
def with_query_value(sender: Sender, cpo_name: str, value):
  return WithQueryValueSender(sender, cpo_name, value)