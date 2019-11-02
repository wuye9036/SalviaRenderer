REM Build salvia only.
python bootstrap.py build salvia --build-root build --install-root . --arch x64 --toolset msvc-14.2 --toolset-dir D:\Software\VS2019\VC\Auxiliary\Build --build-config RelWithDebInfo --cmake D:\Software\CMake\bin\cmake.exe 

REM Launch benchmark test.
python bootstrap.py benchmark --binary-folder bin\ntx64_msvc142\RelWithDebInfo --git "C:\Users\Ye Wu\AppData\Local\Atlassian\SourceTree\git_local\bin\git.exe" --repeat 16
