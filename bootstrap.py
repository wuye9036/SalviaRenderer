#!/usr/bin/env python
import argparse
import functools
import multiprocessing
import os
import shutil
import subprocess
import sys

from blibs import deps
from blibs.benchmark import benchmark_runner
from blibs.copy import copy_newer
from blibs.diagnostic import build_error, report_error, report_info
from blibs.env import add_binpath, systems
from blibs.project import project
from blibs.util import batch_command, executable_file_name, scoped_cd

RESOURCE_COMMIT = "f18fc82afd6ca2f64e4a6eac9275553bc47d42a1"


def guarded_rmtree(path):
    if os.path.isdir(path):
        shutil.rmtree(path)


def make_bjam(prj: project):
    assert isinstance(prj, project)
    src = prj.boost_root()

    with scoped_cd(src):
        bjam_file_name = executable_file_name("bjam", systems.current())

        if os.path.exists(bjam_file_name):
            report_info('Bjam is existed.')
            return True

        customized_toolset_dir = prj.customized_toolset_dir()
        env_vars = os.environ.copy()
        if systems.current() == systems.win32:
            env_var_separator = ';'
            bootstrap_script = "bootstrap.bat"
        else:
            env_var_separator = ':'
            bootstrap_script = "./bootstrap.sh"

        if customized_toolset_dir:
            env_vars['PATH'] = "%s%s%s" % (env_vars['PATH'], env_var_separator, customized_toolset_dir)

        try:
            subprocess.call(bootstrap_script, env=env_vars)
        except OSError:
            report_error("Unknown error occurred when building bjam.")

        if not os.path.exists(bjam_file_name):
            report_error("Bjam wasn't built.")

        return True


def clean_boost(proj):
    src = proj.boost_root()

    os.chdir(src)
    if systems.current() == systems.win32:
        boost_cmd = "bjam --clean"
        print('Executing: %s' % boost_cmd)
        os.system(boost_cmd)
    else:
        return False

    os.chdir(proj.source_root())
    guarded_rmtree(proj.boost_stage())


def make_boost(proj):
    src = proj.boost_root()
    stage = proj.boost_stage()
    
    # Get boost build command
    # Add configs
    libs = ["test", "wave", "program_options", "locale"]
    address_model = f'address-model={proj.arch().bits}'
    options = ["--build-dir=./", "--hash", "link=shared", "runtime-link=shared", "threading=multi", "stage"]
    current_toolset = proj.toolset()
    defs = []
    cxxflags = []
    if current_toolset.short_compiler_name() == 'vc':
        defs = ["_CRT_SECURE_NO_DEPRECATE", "_SCL_SECURE_NO_DEPRECATE"]
        cxxflags = ["-wd4819", "-wd4910", "-wd4244", "-wd4996"]
        
    # bjam current_toolset stagedir address-model defs cxxflags libs options
    bjam_executable = None
    if systems.current() == systems.win32:
        bjam_executable = 'bjam'
    elif systems.current() == systems.linux:
        bjam_executable = './bjam'
    else:
        report_error("Unsupported OS.")

    # configs to cmd
    libs_cmd = ["--with-%s" % lib for lib in libs]
    defs_cmd = []
    if len(defs) > 0:
        defs_cmd = ["define=%s" % dfn for dfn in defs]
    cxxflags_cmd = []
    if len(cxxflags) > 0:
        cxxflags_cmd = ["cxxflags=%s" % flag for flag in cxxflags]

    toolset_cmd = "--current_toolset=%s" % current_toolset.boost_name()
    stage_cmd = '--stagedir=%s' % stage
        
    boost_cmd = [bjam_executable, toolset_cmd, stage_cmd, address_model] \
        + defs_cmd + cxxflags_cmd + libs_cmd + options + proj.boost_configs()
    report_info('Executing: %s' % boost_cmd)

    env_vars = os.environ
    if not proj.customized_toolset_dir() is None:
        env_vars = add_binpath(os.environ, [proj.customized_toolset_dir()])

    boost_build_bat = batch_command(src)
    if proj.toolset().short_compiler_name() == "vc":
        boost_build_bat.add_native_command('@call "%s"' % proj.env_setup_commands())
        boost_build_bat.add_execmd_with_error_exit(" ".join(boost_cmd))
        boost_build_bat.execute(keep_bat=False)
    else:
        os.chdir(src)
        if subprocess.call(boost_cmd, env=env_vars, stdout=sys.stdout) != 0:
            report_error("Boost build failed.")
        os.chdir(proj.source_root())
    report_info("Boost build done.")
    return True


def clean_llvm(proj):
    guarded_rmtree(proj.llvm_build())
    guarded_rmtree(proj.llvm_install())


def config_and_make_cmake_project(project_name, additional_params, source_dir, build_dir, install_dir, proj):
    if additional_params is None:
        conf_params = dict()
    else:
        conf_params = additional_params.copy()
    if proj.toolset().short_compiler_name() != "vc":
        conf_params["CMAKE_BUILD_TYPE"] = ("STRING", proj.config_name())
        conf_params["CMAKE_INFO_ARCH_NAME"] = ("STRING", proj.arch().name)
    if install_dir is not None:
        conf_params["CMAKE_INSTALL_PREFIX"] = ("PATH", install_dir)
    
    params_cmd = functools.reduce(
        lambda cmd, lib: cmd + lib,
        [f' -D {k}:{v[0]}="{v[1]}"' for (k, v) in conf_params.items()]
    )
    
    report_info("Configuring %s ..." % project_name)
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)
    
    conf_cmd = batch_command(build_dir)
    
    if proj.customized_toolset_dir():
        if systems.current() == systems.win32:
            conf_cmd.add_native_command('@set PATH=%s;%%PATH%%' % proj.customized_toolset_dir())
        elif systems.current() == systems.linux:
            conf_cmd.add_native_command('PATH=%s:$PATH' % proj.customized_toolset_dir())
        else:
            report_error("Unsupported OS.")
    conf_cmd.add_execmd_with_error_exit(
        f'"{proj.cmake_exe()}" -G "{proj.generator()}" -A "{proj.arch().name}" {params_cmd} {source_dir}'
    )
    
    if conf_cmd.execute() != 0:
        report_error("%s configure failed." % project_name)
        
    report_info("Making %s on %s ..." % (project_name, proj.config_name()))
    if proj.toolset().short_compiler_name() == "vc":
        make_cmd = batch_command(build_dir)
        make_cmd.add_native_command('@call "%s"' % proj.env_setup_commands())
        make_cmd.add_native_command('@set VisualStudioVersion=%s' % proj.toolset().vs_version_string())
        make_cmd.add_execmd_with_error_exit(
            f'{proj.maker_name()} ALL_BUILD.{proj.project_file_ext()} /m /v:m /p:Configuration={proj.config_name()}'
        )
        
        if install_dir is not None:
            make_cmd.add_execmd_with_error_exit(
                f'{proj.maker_name()} INSTALL.{proj.project_file_ext()} /m /v:m /p:Configuration={proj.config_name()}'
            )
        if make_cmd.execute() != 0:
            report_error("%s make failed." % project_name)
    elif proj.toolset().short_compiler_name() in ["mgw", "gcc"]:
        make_cmd = batch_command(build_dir)
        make_cmd.add_execmd_with_error_exit('%s' % proj.env_setup_commands())
        make_cmd.add_execmd_with_error_exit('%s -j%d' % (proj.maker_name(), multiprocessing.cpu_count()))
        if install_dir is not None:
            make_cmd.add_execmd_with_error_exit('%s install' % proj.maker_name())
        if make_cmd.execute() != 0:
            report_error("%s make failed." % project_name)
    else:
        report_error("Unsupported toolset or OS.")


def config_and_make_llvm(proj):
    # Add definitions here
    configuration = {
        "PYTHON_EXECUTABLE": ("PATH", sys.executable),
        "LLVM_INCLUDE_TESTS": ("BOOL", "FALSE"),
        "LLVM_INCLUDE_TOOLS": ("BOOL", "FALSE"),
        "LLVM_INCLUDE_UTILS": ("BOOL", "FALSE"),
        "LLVM_INCLUDE_EXAMPLES": ("BOOL", "FALSE"),
        "LLVM_INCLUDE_BENCHMARKS": ("BOOL", "FALSE"),
        "LLVM_BUILD_TESTS": ("BOOL", "FALSE"),
        "LLVM_BUILD_TOOLS": ("BOOL", "FALSE"),
        "LLVM_BUILD_UTILS": ("BOOL", "FALSE"),
        "LLVM_TARGETS_TO_BUILD": ("STRING", "X86"),
        # "LLVM_OPTIMIZED_TABLEGEN": ("BOOL", "TRUE")
    }
    config_and_make_cmake_project('LLVM', configuration, proj.llvm_root(), proj.llvm_build(), proj.llvm_install(), proj)


def config_and_make_freeimage(proj):
    config_and_make_cmake_project(
        'FreeImage', None, proj.freeimage_root(), proj.freeimage_build(), proj.freeimage_install(), proj
    )


def config_and_make_freetype(proj):
    config_and_make_cmake_project(
        'FreeType', None, proj.freetype_root(), proj.freetype_build(), proj.freetype_install(), proj
    )


def clean_salvia(proj):
    guarded_rmtree(proj.salvia_build())
    guarded_rmtree(proj.salvia_lib())
    guarded_rmtree(proj.salvia_bin())


def config_and_make_salvia(proj):
    defs = dict()
    defs["SALVIA_BOOST_DIR"] = ("PATH", proj.boost_root())
    defs["PYTHON_EXECUTABLE"] = ("PATH", sys.executable)
    if proj.toolset().short_compiler_name() == 'vc':
        defs["SALVIA_BOOST_LIB_DIR"] = ("PATH", proj.boost_lib_dir_in_msvc())
    else:
        defs["SALVIA_BOOST_LIB_DIR"] = ("PATH", proj.boost_lib_dir())
    if proj.toolset().short_compiler_name() == "vc":
        defs["SALVIA_FREEIMAGE_DIR"] = ("PATH", proj.freeimage_install_in_msvc())
        defs["SALVIA_LLVM_INSTALL_DIR"] = ("PATH", proj.llvm_install_in_msvc())
        defs["SALVIA_FREETYPE_DIR"] = ("PATH", proj.freetype_install_in_msvc())
    else:
        defs["SALVIA_FREEIMAGE_DIR"] = ("PATH", proj.freeimage_install())
        defs["SALVIA_LLVM_INSTALL_DIR"] = ("PATH", proj.llvm_install())
        defs["SALVIA_FREETYPE_DIR"] = ("PATH", proj.freetype_install())
    defs["SALVIA_BUILD_WITH_DIRECTX"] = ("BOOL", "TRUE" if proj.directx() else "FALSE")
    config_and_make_cmake_project('SALVIA', defs, proj.source_root(), proj.salvia_build(), None, proj)


def install_prebuild_binaries(proj):
    report_info("Installing dependencies ...")
    
    dynamic_lib_ext = None
    if proj.os() == systems.win32:
        dynamic_lib_ext = ".dll"
    elif proj.os() == systems.linux:
        dynamic_lib_ext = ".so"
    else:
        report_error("Target OS cannot be supported.")
        
    # Copy mingw runtime
    if proj.toolset().short_compiler_name() == "mgw":
        mgw_dir = proj.customized_toolset_dir()
        libs = ["stdc++", "sjlj", "seh"]
        for f in os.listdir(mgw_dir):
            if os.path.splitext(f)[1] != dynamic_lib_ext:
                continue
            for lib_name in libs:
                if lib_name in f:
                    copy_newer(os.path.join(mgw_dir, f), proj.salvia_bin())

    # Copy FreeImage DLLs
    freeimage_dist_dir = os.path.join(proj.freeimage_install(), "Dist")
    files = os.listdir(freeimage_dist_dir)
    for f in files:
        name, ext = os.path.splitext(f)
        if ext != dynamic_lib_ext:
            continue
        src = os.path.join(freeimage_dist_dir, f)
        copy_newer(src, proj.salvia_bin())

    # Copy d3dcompile_xx.dll
    if proj.dx_dlls_dirs():
        for d in proj.dx_dlls_dirs():
            for f in os.listdir(d):
                copy_newer(os.path.join(d, f), proj.salvia_bin())
    
    # Copy boost DLLs
    files = os.listdir(proj.boost_lib_dir())
    need_copy = []
    for f in files:
        name, ext = os.path.splitext(f)
        if ext != dynamic_lib_ext:
            continue
        if not proj.toolset().boost_lib_name() in name:
            continue
        need_copy.append(os.path.join(proj.boost_lib_dir(), f))

    for f in need_copy:
        copy_newer(f, proj.salvia_bin())


def clean_all(proj):
    print('clean Boost ...')
    clean_boost(proj)
    print('clean LLVM ...')
    clean_llvm(proj)
    print('clean SALVIA ...')
    clean_salvia(proj)
    pass


def build(args):
    proj = project(args, os.getcwd())
    
    inst = deps.installer(RESOURCE_COMMIT, proj.source_root())
    inst.update_all()

    proj.print_props()
    proj.check()

    if proj.is_target_project_enabled('boost'):
        make_bjam(proj)
        make_boost(proj)

    if proj.is_target_project_enabled('freetype'):
        config_and_make_freetype(proj)

    if proj.is_target_project_enabled('freeimage'):
        config_and_make_freeimage(proj)

    if proj.is_target_project_enabled('llvm'):
        config_and_make_llvm(proj)

    if proj.is_target_project_enabled('salvia'):
        config_and_make_salvia(proj)

    install_prebuild_binaries(proj)


def _main():
    parser = argparse.ArgumentParser(
        "Build, test and run benchmark for SALVIA. Please see BUILD_README for more details.")
    subparsers = parser.add_subparsers(dest="command", help="sub-command help")

    parser_build = subparsers.add_parser("build")
    parser_build.add_argument(
        "targets", metavar='TARGETS', type=str, nargs='+',
        help="Specify build targets. "
             "Available targets are: freetype freeimage boost llvm salvia all. "
             "if no target is specified, script just outputs environment variables."
    )
    parser_build.add_argument(
        "--build-root", dest="build_root", type=str, required=True, help="Folder path for build.")
    parser_build.add_argument(
        "--install-root", dest="install_root", type=str, required=True, help="Folder path for placing binaries.")
    parser_build.add_argument("--arch", choices=['x64'], required=True, help="Target architecture.")
    parser_build.add_argument(
        "--toolset", metavar="TOOLSET_NAME", type=str, required=True,
        help="Specify toolset name and version. The name convention is same as Boost.Build. For e.g.: msvc-14.2")
    parser_build.add_argument(
        "--toolset-dir", dest="toolset_dir", type=str, required=True, help="Please see BUILD_README.")
    parser_build.add_argument(
        "--build-config", choices=['Debug', 'RelWithDebInfo'], dest="build_config", type=str, required=True)
    parser_build.add_argument(
        "--cmake", metavar="CMAKE_EXECUTABLE", type=str, required=True, help="Path of CMake executable.")

    parser_benchmark = subparsers.add_parser("benchmark")
    parser_benchmark.add_argument(
        "--git", metavar="GIT_EXECUTABLE", type=str, required=True, help="Path of git executable.")
    parser_benchmark.add_argument(
        "--binary-folder", dest="binary_folder", type=str, required=True, help="Folder of binaries.")
    parser_benchmark.add_argument(
        "--change-desc", dest="change_desc", type=str, help="Description for un-commit changes."
    )

    subparsers.add_parser("clean")

    args = parser.parse_args()

    if args.command == "clean":
        report_info("Cleaning ...")
        proj = project(args, os.getcwd())
        proj.print_props()
        clean_all(proj)
    elif args.command == "build":
        report_info("Building ...")
        try:
            build(args)
            report_info("Salvia building done.")
        except build_error as err:
            print("[E] " + err.message())
            print("[E] Salvia built failed.")
            sys.exit(1)
    elif args.command == "benchmark":
        report_info("Benchmarking ...")
        source_root_dir = os.path.dirname(os.path.abspath(__file__))
        bm_runner = benchmark_runner(source_root_dir, args.binary_folder, args.git)
        bm_runner.run_all(args.change_desc)
    else:
        report_error(f"Command <{args.command}> is unknown..")


if __name__ == "__main__":
    _main()
