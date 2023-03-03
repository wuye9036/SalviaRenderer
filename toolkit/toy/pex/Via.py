from Meta import Args, Sender, Scheduler, Receiver, OperationState, pipeable
from Submit import submit


class ViaValueReceiver(Receiver):
  def __init__(self, args: Args, receiver: Receiver):
    self._args = args
    self._receiver = receiver

  def set_value(self):
    self._args.invoke(self._receiver.set_value)

  def set_done(self):
    self._receiver.set_done()


class ViaDoneReceiver(Receiver):
  def __init__(self, receiver: Receiver):
    self._receiver = receiver

  def set_value(self, *args, **kwargs):
    self._receiver.set_done()

  def set_done(self):
    self._receiver.set_done()


class PredecessorReceiver(Receiver):
  def __init__(self, successor: Sender, receiver: Receiver):
    self._successor = successor
    self._receiver = receiver

  def set_value(self, *args, **kwargs):
    submit(self._successor, ViaValueReceiver(Args(*args, **kwargs), self._receiver))

  def set_done(self):
    submit(self._successor, ViaDoneReceiver(self._receiver))


class ViaSender(Sender):
  def __init__(self, pred: Sender, succ: Sender):
    self._pred_sender = pred
    self._succ_sender = succ

  def connect(self, receiver: Receiver) -> OperationState:
    return self._pred_sender.connect(
      PredecessorReceiver(self._succ_sender, receiver)
    )

  def blocking(self):
    raise NotImplementedError()


def via(scheduler: Scheduler, sender: Sender):
  return ViaSender(sender, scheduler.schedule())
