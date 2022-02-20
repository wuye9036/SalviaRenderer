# Project build guide

## Acknowledgement
  * We would like to thank [JetBrains](https://www.jetbrains.com/?from=salvia) for their donating licenses to their excellent products to develop **Salvia**.

![JetBrains](res/jetbrains.svg)
  
  * In this project we are using:
    * PyCharm
    * CLion
    * Reshaper C++

## Requirements
  * Windows 10
    * Visual Studio 2019.
    * Python 3.5 or later
    * CMake 3.21+
  * Linux
    * Mint 16 and Mint 17 were tested.
    * GCC 8 or later.
    * Python 3.5 or later
    * CMake 3.15 or later
    
## Build steps
  * Install [vcpkg](https://github.com/microsoft/vcpkg).
  * Edit SAMPLE_Build.bat.
  * Run the modified script.

 
## REMARK
  * While `bootstrap.py` build was run, only one configuration was avaiable for build. For IDEs which support multiple configurations, such as Visual Studio, you need to run bootstrap several times to make the configurations available for building.
  
  
## Support Info
  * If you have any question, please contact: wuye9036 _at_ gmail _dot_ com
