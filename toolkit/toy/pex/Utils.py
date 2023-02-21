class IndentHistory:
  def __init__(self):
    self._indent = -1
    self._instance = None

  def inc(self, indent="  "):
    self._indent += 1
    return indent * self._indent

  def dec(self):
    self._indent -= 1


default_indent = IndentHistory()


def trace_func(fn=None, *, prefix="", postfix="", ending="", indent_hist=default_indent, indent="  "):
  def _trace_wrapper(func):
    def _trace(*args, **kwargs):
      current_indent = indent_hist.inc(indent)
      print(f"{current_indent}{prefix}invoke: {func.__qualname__} {postfix}")
      ret = func(*args, **kwargs)
      indent_hist.dec()
      print(ending, end="")
      return ret

    return _trace

  if fn is None:
    return _trace_wrapper
  else:
    return _trace_wrapper(fn)


trace_test = trace_func(prefix="### ### ###\n", ending="===\n\n")
