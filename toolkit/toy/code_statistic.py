import sys
import pathlib
import dataclasses

@dataclasses.dataclass
class Options:
  IncludeList = []
  ExcludeList = set()
  ExtensionFilter = set()

@dataclasses.dataclass(repr=True)
class StatResult:
  FileCount: int = 0
  FileSizeTotal: int = 0
  FileLines: int = 0

  NullLines: int = 0
  EmptyLines: int = 0
  SingleLineComments: int = 0
  MultiLineComments: int = 0
  PreprocessorLines: int = 0
  CodeLines: int = 0
  MixedLines: int = 0

def IsEmptyLine(l: str):
  return not any(ch.isalpha() or ch.isdigit() for ch in l)

def ParseFile(fPath: pathlib.Path, result: StatResult):
  result.FileCount += 1
  result.FileSizeTotal += fPath.stat().st_size

  lines = fPath.read_text(encoding="utf-8").splitlines()
  
  in_multiline_comment = False
  for l in lines:
    stripped_l = l.strip()
    if not in_multiline_comment:
      if stripped_l.startswith("#"):
        result.PreprocessorLines += 1
        continue
      if stripped_l.startswith( "//" ):
        result.SingleLineComments += 1
        continue
      if stripped_l.find("/*") != -1:
        in_multiline_comment = True
        if stripped_l[:2] == "/*" != -1:
          result.MultiLineComments += 1
        else:
          result.MixedLines += 1
      else:
        if stripped_l == "":
          result.NullLines += 1
        elif IsEmptyLine(stripped_l):
          result.EmptyLines += 1
        else:
          result.CodeLines += 1
    else:
      if stripped_l.find("*/") != -1:
        in_multiline_comment = False
        if stripped_l[-2:] == "*/":
          result.MultiLineComments += 1
        else:
          result.MixedLines += 1
      else:
        result.MultiLineComments += 1
  
def Scan(path: pathlib.Path, opt: Options, result: StatResult):
  for child in path.iterdir():
    assert isinstance(child, pathlib.Path)

    child_s = child.resolve().as_posix()
    if child_s in opt.ExcludeList:
      # print(f"! Folder {child.resolve()} was excluded.")
      continue

    if child.is_dir():  
      Scan(child, opt, result)
    else:
      if child.suffix in opt.ExtensionFilter:
        ParseFile(child, result)

def main():
  root = pathlib.Path(".").resolve()
  print(f"Root folder: {root}")

  opt = Options()

  opt.IncludeList = [
    root / "salvia",
    root / "samples",
    root / "sasl",
    root / "eflib"
  ]

  opt.ExcludeList = set(
    p.resolve().as_posix() for p in [
      root / "eflib" / "include" / "eflib" / "math" / "write_mask",
      root / "eflib" / "include" / "eflib" / "math" / "swizzle",
      root / "eflib" / "include" / "eflib" / "concurrency" / "thread_pool",
      root / "sasl" / "include" / "sasl" / "enums",
      root / "sasl" / "src" / "enums"
    ]
  )
  opt.ExtensionFilter = set(
    ".h;.cpp".split(";")
  )

  result = StatResult()

  for fd in opt.IncludeList:
    # result = StatResult()
    # print(f">>> {fd.resolve().as_posix()}")
    Scan(fd, opt, result)
    # print(f"<<< {result}")
  
  print(f"{result}")
  
        
if __name__ == "__main__":
  main()