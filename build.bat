set CMAKE_EXE="D:\Software\CMake\bin\cmake.exe"
set VCPKG_CMAKE="E:\Code\Library\vcpkg\scripts\buildsystems\vcpkg.cmake"
set CMAKE_GENERATOR="Visual Studio 17 2022"
set CMAKE_BUILD_TYPE=RelWithDebInfo
set CMAKE_GENERATOR_PLATFORM=x64
set BUILD_FOLDER=./build/msvc_x64_win
%CMAKE_EXE% -B %BUILD_FOLDER% -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_CMAKE%
%CMAKE_EXE% --build %BUILD_FOLDER% --config RelWithDebInfo