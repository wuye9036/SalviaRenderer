def chain(snd, *snd_fact_and_args):
  for fact_n_arg in snd_fact_and_args:
    snd = fact_n_arg[0](snd, *fact_n_arg[1:])
  return snd


def continuation_style(f):
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
      return Sender(f(*args, **kwargs))
    return PartialSender(f, *args, **kwargs)

  return _impl


class PartialSender:
  def __init__(self, f, *args, **kwargs) -> None:
    self._f = f
    self._args = args
    self._kwargs = kwargs

  def apply(self, sender):
    assert isinstance(sender, Sender)
    return Sender(self._f(sender, *self._args, **self._kwargs))


class Sender:
  def __init__(self, impl) -> None:
    self._impl = impl

  def __getattr__(self, __name: str):
    return getattr(self._impl, __name)

  def __call__(self, *args, **kwargs):
    self._impl(*args, **kwargs)

  def __or__(self, partial_sender: PartialSender):
    assert isinstance(partial_sender, PartialSender)
    return partial_sender.apply(self)


class Args:
  def __init__(self, *args, **kwargs) -> None:
    self._args = args
    self._kwargs = kwargs

  def invoke(self, fn, *prior_args):
    return fn(*prior_args, *self._args, **self._kwargs)
