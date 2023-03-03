import abc
import typing
import enum


def chain(snd, *snd_fact_and_args):
  for fact_n_arg in snd_fact_and_args:
    snd = fact_n_arg[0](snd, *fact_n_arg[1:])
  return snd


def pipeable(f):
  """
  Decorator. Added continuation form to the execution algorithm. For e.g.
  ```
  @algo_as_continuation
  def then(sender, fn):
    pass
  ```
  could be used as:
  `then(then(sch.schedule(), f0), f1)`
  OR
  `then(f1)(then(f0)(sch.schedule()))`

  Or more human-readable by operator overloading:

  ```
  sch.schedule() | then(f0) | then(f1)
  ```
  """
  def _impl(*args, **kwargs):
    if isinstance(args[0], Sender):
      # algo(Sender, ...) -> Sender
      return f(*args, **kwargs)
    return BindBack(f, *args, **kwargs)

  return _impl


def customizable(fn):
  def _impl(obj, *args, **kwargs):
    fn_name = fn.__name__
    if fn_name in obj.customization_table:
      return obj.customization_table[fn_name](*args, **kwargs)
    else:
      return fn(obj, *args, **kwargs)
  return _impl


class Pipeable:
  def __or__(self, other):
    return other(self)


class AsPipeable(Pipeable):
  def __init__(self, inner):
    self._inner = inner

  def __getattr__(self, __name: str):
    return getattr(self._impl, __name)

  def __call__(self, *args, **kwargs):
    self._inner(*args, **kwargs)


def as_pipeable(inner_object):
  return AsPipeable(inner_object)


class BindBack:
  def __init__(self, f, *args, **kwargs) -> None:
    self._f = f
    self._args = args
    self._kwargs = kwargs

  def __call__(self, predecessor: Pipeable):
    assert isinstance(predecessor, Pipeable)
    ret = self._f(predecessor, *self._args, **self._kwargs)
    assert isinstance(ret, Pipeable)
    return ret


class OperationState:
  @abc.abstractmethod
  def start(self):
    pass


class Receiver:
  @abc.abstractmethod
  def set_value(self, *args, **kwargs):
    pass

  @abc.abstractmethod
  def set_done(self):
    pass

  def get_scheduler(self) -> 'Scheduler':
    raise ValueError()


class SubmitNotImplemented:
  pass


class BlockingKind(enum.Enum):
  Maybe = "Maybe"
  Never = "Never"
  Always = "Always"
  AlwaysInline = "AlwaysInline"


class Sender(Pipeable):
  @abc.abstractmethod
  def connect(self, receiver: Receiver) -> OperationState:
    pass


class HasSubmit:
  @abc.abstractmethod
  def submit(self, receiver: Receiver):
    pass


class Scheduler:
  @abc.abstractmethod
  def schedule(self) -> Sender:
    pass


SenderDecorator = typing.Callable[[Sender], Sender]


class Args:
  def __init__(self, *args, **kwargs) -> None:
    self._args = args
    self._kwargs = kwargs

  def invoke(self, fn, *prior_args):
    return fn(*prior_args, *self._args, **self._kwargs)


# Placeholder class for reduce operations.
class Unit:
  pass

