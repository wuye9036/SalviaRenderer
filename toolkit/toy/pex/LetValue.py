from Utils import trace_func
from Meta import Args


class let_value_op:
  def __init__(self, pred, succFact, receiver) -> None:
    self.succFact = succFact
    self.receiver = receiver
    self._predOp = pred.connect(let_value_pred_receiver(self))
    self._values = Args()

  def start(self):
    self._predOp.start()


class let_value_pred_sender:
  def __init__(self, pred, succFact):
    self._pred = pred
    self._succFact = succFact

  @trace_func
  def connect(self, receiver):
    return let_value_op(self._pred, self._succFact, receiver)


class let_value_succ_receiver:
  def __init__(self, op):
    self._op = op

  @trace_func
  def set_value(self, *args, **kwargs):
    self._op.receiver.set_value(*args, **kwargs)


class let_value_pred_receiver:
  def __init__(self, op):
    self._op = op

  @trace_func
  def set_value(self, *args, **kw_args):
    succ_factory_args = Args(*args, **kw_args)
    succ_operation = succ_factory_args.invoke(self._op.succFact).connect(let_value_succ_receiver(self._op))
    succ_operation.start()


def let_value(pred, succFact):
  return let_value_pred_sender(pred, succFact)

