
class config:
  def __init__(self):
    self.trace_func_enabled = True
    self.trace_test_enabled = True

  @staticmethod
  def get():
    return _cfg


_cfg = config()


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


def trace_func(
    fn=None,
    *,
    prefix="", postfix="", ending="",
    indent_hist=default_indent, indent="  ",
    is_trace_test: bool = False
):
  def _trace_wrapper(func):
    def _trace(*args, **kwargs):
      print_on = (
          config.get().trace_func_enabled and (
              not is_trace_test or
              (is_trace_test and config.get().trace_test_enabled)))

      current_indent = indent_hist.inc(indent)
      if print_on: print(f"{current_indent}{prefix}invoke: {func.__qualname__} {postfix}")
      ret = func(*args, **kwargs)
      indent_hist.dec()
      if print_on: print(ending, end="")
      return ret

    return _trace

  if fn is None:
    return _trace_wrapper
  else:
    return _trace_wrapper(fn)


trace_test = trace_func(prefix="### ### ###\n", ending="===\n\n", is_trace_test=True)
