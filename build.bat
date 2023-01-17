@REM ---------------------------------------
@REM Configure the path to your cmake and vcpkg.
@set CMAKE_EXE="cmake.exe"
@set VCPKG_CMAKE="E:\Code\Library\vcpkg\scripts\buildsystems\vcpkg.cmake"
@REM ---------------------------------------


@REM ---------------------------------------
@REM Configure the build parameters.
@set SALVIA_GENERATOR="Visual Studio 17 2022"
@set SALVIA_CONFIG=RelWithDebInfo
@set SALVIA_PLATFORM=x64
@REM ---------------------------------------


@REM ---------------------------------------
@REM Configure your build and install folder
@set SALVIA_BUILD=..\salvia.msvc1930_win64_rel.build
@set SALVIA_INSTALL=..\salvia.msvc1930_win64_rel.redist
@REM ---------------------------------------


@REM ---------------------------------------
@REM Build and Install.
%CMAKE_EXE% -G %SALVIA_GENERATOR% -B %SALVIA_BUILD% -S . -DCMAKE_BUILD_TYPE=%SALVIA_CONFIG% -A %SALVIA_PLATFORM% -DCMAKE_TOOLCHAIN_FILE=%VCPKG_CMAKE%
%CMAKE_EXE% --build %SALVIA_BUILD% --config %SALVIA_CONFIG%
%CMAKE_EXE% --install %SALVIA_BUILD% --config %SALVIA_CONFIG% --prefix %SALVIA_INSTALL%
@REM ---------------------------------------