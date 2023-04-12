# Project build guide

## Acknowledgement
  * We would like to thank [JetBrains](https://www.jetbrains.com/?from=salvia) for their donating licenses to their excellent products to develop **Salvia**.

![JetBrains](res/jetbrains.svg)
  
  * In this project we are using:
    * PyCharm
    * CLion
    * Reshaper C++

## Requirements and build steps

### Prerequisites for **ALL** platforms

* Git with [LFS](https://git-lfs.github.com/)
* CMake 3.23+
* [vcpkg](https://vcpkg.io/en/)

**Note:** CMake and Git may installed with OS, IDEs or build essentials. Please check the installed versions and the activated toolchains.

### Windows 10/11
1. Install latest [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) with C++, CMake, and Git.
2. Install [vcpkg](https://vcpkg.io/en/) side-by-side with source code. For example, if your project is located at `D:\path\salvia`, then vcpkg should be installed at `D:\path\vcpkg`. Otherwise, you need to specify the vcpkg toolchain file by the config `_windows/toolchainFile` in `CMakePreset_Woodblocks.json`.
3. Open the folder `D:\path\salvia` with Visual Studio 2022
4. Select the preset `msvc_dbg_win` or `msvc_rel_win` in the CMake Presets tab and build it.

### Linux/WSL

1. `gcc` above 12.0 or `clang` above 14.0 is required
2. Clone the project to `path-to-salvia`.
3. Install [vcpkg](https://vcpkg.io/en/) by your OS package system (for e.g. `apt` on ubuntu). The default installation path of vcpkg is at `~/vcpkg`. If it was installed elsewhere, you need to specify the vcpkg toolchain file by the config `_linux/toolchainFile` in `CMakePreset_Woodblocks.json`.
4. Open the folder `path-to-salvia` with your IDE (VSCode, VIM or CLion).

**NOTE for CLion on WSL**: If you are working on WSL with CLion, please make sure that the correct CMake and Git and compiler in CLion was selected. You can check or edit the toolchains in `Settings > Build, Execution, Deployment > Toolchains`.

### Mac

1. Install latest XCode Commad Line Tools.
2. `gcc` above 12.0 or `clang` above 14.0 is required.
3. Clone the project to `path-to-salvia`.
4. Install [vcpkg](https://vcpkg.io/en/) by your OS package system (for e.g. `brew` on Mac). The default installation path of vcpkg is at `~/Library/vcpkg`. If it was installed elsewhere, you need to specify the vcpkg toolchain file by the config `_macos/toolchainFile` in `CMakePreset_Woodblocks.json`.
5. Open the folder `path-to-salvia` with your IDE (VSCode, VIM or CLion).

**NOTE for CLion on Mac**: Please download M1 version of CLion if you are using M1 Mac. Otherwise, you may encounter some issues when building the project.

## Suggestions for IDEs

* Windows: We recommend using Visual Studio 2022 and open *salvia* as folder. VSCode is also available, but only the official C++ intellisense plugin was supported when build with MSVC and its experience is relatively bad.
* Linux/WSL: CLion is our recommended IDE. If you are using VSCode, we suggest use clangd as the language server. For your convience, the build script generates `compile_commands.json` and copy to root for clangd by default. And we also highly recommend you to install `clang-tidy` and `clang-format` for better code quality.
* Mac: Same as Linux.

## Known issues

All interactive demos are not supported on Linux and Mac because we don't have GUI support on these platforms.

## Support Info

If you have any question, please contact: wuye9036 _at_ gmail _dot_ com
