# Project build guide

## Acknowledgement
  * We would like to thank [JetBrains](https://www.jetbrains.com/?from=salvia) for their donating licenses to their excellent products to develop **Salvia**.

![JetBrains](res/jetbrains.svg)
  
  * In this project we are using:
    * PyCharm
    * CLion
    * Reshaper C++

## Requirements
  * Windows 10/11
    * Git
      * Need to install LFS plug-in.
      * [About Git LFS](https://git-lfs.github.com/)
    * Visual Studio 2022
      * Need C++20 support.
    * CMake 3.21+
    * vcpkg (See build steps)
  * Linux
    * Clang-14.0 or higher
    * stdlibc++ 11 (It may be distributed with same version of GCC) or higher.
    * *All samples cannot be run in Linux*. Will be supported in 2023.
    * Our build & test environment is WSL2 + Ubuntu 22.04 LTS
    
## Build steps
  * Install [vcpkg](https://github.com/microsoft/vcpkg).
  * Edit SAMPLE_Build.bat.
  * Run the modified script.

## Support Info
  * If you have any question, please contact: wuye9036 _at_ gmail _dot_ com
