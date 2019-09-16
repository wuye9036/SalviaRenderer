import sys
import os
from functools import reduce
from .env import arch, systems, detect_cmake, detect_gcc, windows_kit_dirs, toolset
from .diagnostic import report_error, report_warning
from .boost_build import boost_version


class project:
    def __init__(self, props, cwd):
        self.props_ = props

        self.boost_ver_ = None
        self.build_root_ = props.build_root
        self.install_root_ = props.install_root
        self.arch_ = arch.from_machine(props.arch)
        self.os_ = systems.current()
        self.config_ = props.build_config
        self.target_projects_ = props.targets
        self.cwd_ = cwd

        self.cmake_ = detect_cmake(props.cmake)
        env_vars = os.environ
        default_toolset = props.toolset
        candidate_gcc_dir = None
        try:
            candidate_gcc_dir = props.toolset_dir
        except AttributeError:
            pass
        default_gcc, default_gcc_dir = detect_gcc(candidate_gcc_dir, 4, 7)

        # vs_installers = detect_vs_installer()

        if default_toolset is None or default_toolset == "":
            if "VS140COMNTOOLS" in env_vars:
                default_toolset = "msvc-14.0"
            elif default_gcc:
                # gcc or mingw
                default_toolset = "gcc"
            else:
                report_error('Cannot found any valid compiler on your computer. Please set toolset in "proj.py" ')

        if default_toolset == "msvc-14.2":
            self.toolset_ = toolset('vs', 'msvc', 14, 2, None)
            self.builder_root_ = ""
        elif default_toolset == "gcc" and default_gcc is not None:
            self.toolset_ = default_gcc
            self.builder_root_ = default_gcc_dir
        else:
            report_error(f'Unsupported toolset name <{default_toolset}>.')

        self.use_win_kit_ = self.toolset_.short_compiler_name() == "vc" and self.toolset().major_ver >= 11
        self.directx_ = (self.toolset_.short_compiler_name() == "vc") and ("DXSDK_DIR" in env_vars or self.use_win_kit_)
        self.dx_dlls_dirs_ = None

        if self.use_win_kit_:
            wdk_dirs = windows_kit_dirs()
            arch_subdir = None
            if self.arch_ == arch.x64:
                arch_subdir = "x64"
            else:
                report_error("Unsupported arch when finding Windows Kits.")
            self.dx_dlls_dirs_ = [os.path.join(wdk_dir, "Redist", "D3D", arch_subdir) for wdk_dir in wdk_dirs]

    def check(self):
        if self.cmake_ is None:
            report_error("CMake is not available. Please install CMake and set proj.py correctly.")
        if not self.boost_version():
            report_error("Boost is not found. Please specify boost path in \'proj.py\'.")
        if self.toolset().short_compiler_name() == "mgw" and self.arch() == arch.x64:
            if not self.toolset().support_x64():
                report_error('Salvia cannot build as 64-bit when MinGW32 is used.')
        if self.arch() != arch.current() and systems.current() != systems.win32:
            report_warning(
                "In some systems such as linux, cross compiling may cause some errors. "
                "We suggest that you should use same arch for compiling as system's."
            )
        if self.toolset().short_compiler_name() in ['mgw', 'gcc'] \
                and self.customized_toolset_dir() != self.props_.toolset_dir:
            used_toolset_dir = \
                "system-specified path" if self.customized_toolset_dir() is None else self.customized_toolset_dir()
            report_warning(f"Item 'toolset_dir' is set but not used. The used toolset dir is <{used_toolset_dir}>")

    def print_props(self):
        print('=' * 25 + ' Checking Project Properties ' + '=' * 25)
        print(' * Target Projects ......... %s' % ",".join(self.target_projects_))
        print(' * Current Arch ............ %s' % arch.current().name)
        print(' * Current OS .............. %s' % systems.current().name)
        print(' * Toolset ................. %s' % self.toolset().short_name())
        print(' * Env Script(win32 only) .. %s' % os.path.normpath(self.env_setup_commands()))
        print(' * CMake Executable ........ %s' % self.cmake_exe())
        print(' * CMake Generator ......... %s' % self.generator())
        print(' * Make tool ............... %s' % self.maker_name())
        print(' * Use Windows Kits ........ %s' % self.use_win_kit_)
        print(' * Use Direct3D ............ %s' % self.directx_)
        print(' ')
        print(' * Target .................. %s' % self.target_modifier(['platform', 'tool', 'config']))
        print(' * Source .................. %s' % self.source_root())
        print(' * Build ................... %s' % self.build_root())
        print(' * Install ................. %s' % self.install_root())
        print(' * Install Binaries ........ %s' % self.install_bin())
        print(' * Install Libraries ....... %s' % self.install_lib())
        print(' ')
        print(' * Boost ................... %s' % self.boost_root())
        print(' * Boost Version ........... %s' % self.boost_version())
        print(' * Boost Stage ............. %s' % self.boost_stage())
        print(' * LLVM .................... %s' % self.llvm_root())
        print(' * LLVM Build .............. %s' % self.llvm_build())
        print(' * LLVM Install ............ %s' % self.llvm_install())
        print(' * FreeImage ............... %s' % self.freeimage_root())
        print(' * FreeImage Build ......... %s' % self.freeimage_build())
        print(' * FreeImage Install ....... %s' % self.freeimage_install())
        print(' * FreeType2 ............... %s' % self.freetype_root())
        print(' * FreeType2 Build ......... %s' % self.freetype_build())
        print(' * FreeType2 Install ....... %s' % self.freetype_install())
        print(' ')
        print(' * SALVIA Build ............ %s' % self.salvia_build())
        print(' * SALVIA Binaries ......... %s' % self.salvia_bin())
        print("========== Project Properties ==========")
        
    def arch(self):
        return self.arch_

    def os(self):
        return self.os_

    def toolset(self) -> toolset:
        return self.toolset_

    def customized_toolset_dir(self):
        if self.toolset().short_compiler_name() == "msvc":
            return None
        if self.toolset().short_compiler_name() in ["mgw", "gcc"]:
            return self.builder_root_

    def cmake_exe(self):
        return self.cmake_

    def target_modifier(self, hints):
        hint_dict = dict()
        hint_dict["os"] = self.os().name
        hint_dict["arch"] = self.arch().name
        hint_dict["tool"] = self.toolset().short_name()
        hint_dict["platform"] = self.os().name + self.arch().name
        hint_dict["config"] = self.config_
        return reduce(lambda ret, s: ret + "_" + s, [hint_dict[hnt] for hnt in hints])

    def env_setup_commands(self):
        major_ver = self.toolset().major_ver
        minor_ver = self.toolset().minor_ver
        
        if self.os() == systems.win32:
            if self.toolset().short_compiler_name() == 'vc':
                assert major_ver > 14 or (major_ver == 14 and minor_ver >= 2)
                return os.path.join(self.props_.toolset_dir, r"vcvars64.bat")
            elif self.toolset().short_compiler_name() == "mgw":
                if self.build_root_ is not None:
                    return 'set PATH=%s;%%PATH%%' % self.builder_root_
        if self.os() == systems.linux and self.build_root_ is not None:
            return 'PATH=%s:$PATH' % self.builder_root_
        report_error("Unrecognized OS or toolset.")
        
    def maker_name(self):
        if self.toolset().short_compiler_name() == 'vc':
            return 'MSBuild'
        elif self.toolset().short_compiler_name() == "mgw":
            return 'mingw32-make'
        elif self.toolset().short_compiler_name() == "gcc":
            return 'make'
        else:
            report_error('Cannot find valid make/build program.')
    
    def directx(self):
        return self.directx_

    def dx_dlls_dirs(self):
        return self.dx_dlls_dirs_

    def config_name(self):
        return self.config_
    
    def msvc_platform_name(self):
        platform_name_in_msvc = ""
        if self.arch() == arch.x86:
            platform_name_in_msvc = "Win32"
        elif self.arch() == arch.x64:
            platform_name_in_msvc = "x64"
        return platform_name_in_msvc
        
    def project_file_ext(self):
        if self.toolset_.short_compiler_name() == "vc":
            return "vcxproj"
        
    def msvc_config_name_with_platform(self):
        return '"' + self.config_ + "|" + self.msvc_platform_name() + '"'

    def to_abs(self, path):
        if os.path.isabs(path):
            return path
        else:
            return os.path.abspath(os.path.join(self.source_root(), path))

    def source_root(self):
        source_root_path = os.path.dirname(sys.argv[0])
        if os.path.isabs(source_root_path):
            return source_root_path
        else:
            return os.path.abspath(os.path.join(self.cwd_, source_root_path))

    def build_root(self):
        return self.to_abs(self.build_root_)

    def install_root(self):
        return self.to_abs(self.install_root_)

    def install_bin(self):
        return os.path.join(self.install_root(), "bin")

    def install_lib(self):
        return os.path.join(self.install_root(), "lib")

    # Boost builds.
    def boost_root(self):
        return os.path.join(self.source_root(), "3rd_party", "boost")

    def boost_version(self):
        if self.boost_ver_ is None:
            self.boost_ver_ = boost_version(self.boost_root())
        return self.boost_ver_

    def boost_stage(self):
        return self.common_install_dir('boost')

    def boost_configs(self):
        if self.config_ == "Debug":
            return ["optimization=off", "debug-symbols=on", "inlining=off", "runtime-debugging=on"]
        elif self.config_ == "RelWithDebInfo":
            return ["optimization=speed", "debug-symbols=on", "inlining=full", "runtime-debugging=off"]
        elif self.config_ == "Release":
            return ["optimization=speed", "debug-symbols=off", "inlining=full", "runtime-debugging=off"]
        elif self.config_ == "MinSizeRel":
            return ["optimization=space", "debug-symbols=off", "inlining=on", "runtime-debugging=off"]
        else:
            report_error("Configuration <%s> cannot be recognized." % self.config_)

    def boost_lib_dir(self):
        return os.path.join(self.boost_stage(), "lib")

    def boost_lib_dir_in_msvc(self):
        return os.path.join(self.common_msvc_install_dir('boost'), 'lib')

    def common_msvc_install_dir(self, lib_name):
        if self.toolset().short_compiler_name() == 'vc':
            return os.path.join(self.install_lib(),
                                lib_name + "_" + self.target_modifier(['platform', 'tool']) + '_$(ConfigurationName)')
        report_error("Toolset is not set or not MSVC.")

    def common_install_dir(self, lib_name):
        return os.path.join(self.install_lib(), lib_name + "_" + self.target_modifier(['platform', 'tool', 'config']))

    def common_build_dir(self, lib_name):
        if self.toolset().short_compiler_name() == "vc":
            return os.path.join(self.build_root(), lib_name + "_" + self.target_modifier(['platform', 'tool']))
        return os.path.join(self.build_root(), lib_name + "_" + self.target_modifier(['platform', 'tool', 'config']))

    def llvm_root(self):
        return os.path.join(self.source_root(), "3rd_party", "llvm")

    def llvm_build(self):
        return self.common_build_dir("llvm")

    def llvm_install(self):
        return self.common_install_dir("llvm")

    def llvm_install_in_msvc(self):
        return self.common_msvc_install_dir("llvm")

    def freeimage_root(self):
        return os.path.join(self.source_root(), "3rd_party", "FreeImage")

    def freeimage_build(self):
        return self.common_build_dir('freeimage')

    def freeimage_install(self):
        return self.common_install_dir('freeimage')

    def freeimage_install_in_msvc(self):
        return self.common_msvc_install_dir('freeimage')

    def freetype_root(self):
        return os.path.join(self.source_root(), "3rd_party", "freetype2")

    def freetype_build(self):
        return self.common_build_dir('freetype')

    def freetype_install(self):
        return self.common_install_dir('freetype')

    def freetype_install_in_msvc(self):
        return self.common_msvc_install_dir('freetype')

    def salvia_build(self):
        return self.common_build_dir("salvia")

    def salvia_lib(self):
        return os.path.join(self.install_lib(), self.target_modifier(['platform', 'tool']),
                            self.target_modifier(['config']))

    def salvia_bin(self):
        return os.path.join(self.install_bin(), self.target_modifier(['platform', 'tool']),
                            self.target_modifier(['config']))

    def generator(self):
        if self.toolset().short_compiler_name() == "mgw":
            return "CodeBlocks - MinGW Makefiles"
        elif self.toolset().short_compiler_name() == "gcc":
            return "CodeBlocks - Unix Makefiles"
        elif self.toolset().short_compiler_name() == 'vc':
            assert self.arch() == arch.x64
            if self.toolset().short_name() == "msvc142":
                return "Visual Studio 16 2019"

        report_error("Unknown generator.")

    def target_projects(self):
        return self.target_projects_

    def is_target_project_enabled(self, proj_name):
        if proj_name in self.target_projects_:
            return True
        if 'all' in self.target_projects_:
            return True
        return False
