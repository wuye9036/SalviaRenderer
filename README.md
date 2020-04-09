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
    * CMake 3.15+
  * Linux
    * Mint 16 and Mint 17 were tested.
    * GCC 8 or later.
    * Python 3.5 or later
    * CMake 3.15 or later
    
## Build steps
  * Running bootstrap.py for build, running benchmark or generating benchmark report CSV file. Example command lines:
      ** Build: `python bootstrap.py build salvia --build-root .\build --install-root . --arch x64 --toolset msvc-14.2 --toolset-dir D:\Software\VS2019\VC\Auxiliary\Build --build-config RelWithDebInfo --cmake D:\Software\CMake\bin\cmake.exe`
      ** Run benchmark: `python bootstrap.py benchmark --binary-folder bin\ntx64_msvc142\RelWithDebInfo --git D:\Software\Git\git.exe --change-desc "Performance improved" --repeat 16`
      ** Generate benchmark report: `python bootstrap.py bm_report`
   * Run `python bootstrap.py --help` to get more details.

 
## REMARK
  * While `bootstrap.py` build was run, only one configuration was avaiable for build. For IDEs which support multiple configurations, such as Visual Studio, you need to run bootstrap several times to make the configurations available for building.
  
  
## Support Info
  * If you have any question, please contact: wuye9036 _at_ gmail _dot_ com
