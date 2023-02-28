from Meta import Sender, Receiver, OperationState


class SeqOperation(OperationState):
  def __init__(self, pred: Sender, succ: Sender, receiver):
    self._predecessor = pred
    self._predOp = pred.connect(SeqPredecessorReceiver(self))
    self.successor = succ
    self.succOp = None
    self.receiver = receiver

  def start(self):
    self._predOp.start()


class SeqSuccessorReceiver(Receiver):
  def __init__(self, op: SeqOperation):
    self._op = op

  def set_value(self, *args, **kwargs):
    self._op.receiver.set_value(*args, **kwargs)

  def set_done(self):
    self._op.receiver.set_done()


class SeqPredecessorReceiver(Receiver):
  def __init__(self, op: SeqOperation):
    self._op = op

  def set_value(self, *args, **kwargs):
    op = self._op
    op.succOp = op.successor.connect(SeqSuccessorReceiver(op))
    op.succOp.start()

  def set_done(self):
    op = self._op
    op.receiver.set_done()


class SeqSender(Sender):
  def __init__(self, pred, succ):
    self._pred = pred
    self._succ = succ

  def connect(self, receiver: Receiver) -> OperationState:
    return SeqOperation(self._pred, self._succ, receiver)


def sequence(*args):
  assert len(args) > 0
  if len(args) == 1:
    return args[0]
  if len(args) == 2:
    return SeqSender(0, 1)
  else:
    return sequence(sequence(args[0], args[1]), *args[2:])