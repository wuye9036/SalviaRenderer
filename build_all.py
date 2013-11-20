#!/usr/bin/env python
#-*- coding:utf-8 -#-

import os, sys, re, atexit, getopt, subprocess, multiprocessing
from functools import *

from blibs import *
from blibs.copy import *
from blibs.env import *
from blibs.util import *
from blibs.project import *


def guarded_rmtree(path):
	if os.path.isdir(path):
		shutil.rmtree(path)

log_f = None

def close_log():
	global log_f
	if not log_f is None:
		log_f.close()

def make_bjam(prj):
	assert isinstance(prj, project)
	old_dir = os.curdir
	src = prj.boost_root()
	os.chdir(src)

	bjam_file_name = executable_file_name("bjam", systems.current())

	if os.path.exists(bjam_file_name):
		report_info('Bjam is existed.')
		os.chdir(old_dir)
		return True

	customized_toolset_dir = prj.customized_toolset_dir()
	if customized_toolset_dir:
		env = os.environ.copy()
		try:
			if systems.current() == systems.win32:
				env['PATH'] = "%s;%s" % (env['PATH'], customized_toolset_dir)
				subprocess.call("bootstrap.bat", env=env)
			else:
				env['PATH'] = "%s:%s" % (env['PATH'], customized_toolset_dir)
				subprocess.call("bootstrap.sh", env=env)
		except:
			os.chdir(old_dir)
			report_error("Unknown error occurred when building bjam.")

	if not os.path.exists(bjam_file_name):
		os.chdir(old_dir)
		report_error("Bjam wasn't built.")

	os.chdir(old_dir)
	return True

def clean_boost(proj):
	src = proj.boost_root()
	stage = proj.boost_stage()

	os.chdir(src)
	if systems.current() == systems.win32:
		boost_cmd = "bjam --clean"
		print( 'Executing: %s' % boost_cmd )
		os.system(boost_cmd)
	else:
		return False

	os.chdir(proj.source_root())
	guarded_rmtree(proj.boost_stage())

def make_boost(proj):
	src = proj.boost_root()
	stage = proj.boost_stage()

	os.chdir(src)
	#Get boost build command
	if systems.current() == systems.win32:
		# Add configs
		libs = ["atomic", "chrono", "thread", "system", "filesystem", "date_time", "test", "wave", "program_options",
				"serialization", "locale"]
		address_model = 'address-model=%d' % proj.arch().bits()
		options = "--build-dir=./ link=shared runtime-link=shared threading=multi stage"
		toolset = proj.toolset()
		defs = []
		cxxflags = []
		if toolset.short_compiler_name() == 'vc':
			defs = ["_CRT_SECURE_NO_DEPRECATE", "_SCL_SECURE_NO_DEPRECATE"]
			cxxflags = ["-wd4819", "-wd4910"]

		#configs to cmd
		libs_cmd = reduce(lambda cmd, lib: cmd + lib, ["--with-%s " % lib for lib in libs])
		defs_cmd = ""
		if len(defs) > 0:
			defs_cmd = reduce(lambda cmd, dfn: cmd + dfn, ["define=%s " % dfn for dfn in defs])
		cxxflags_cmd = ""
		if len(cxxflags) > 0:
			cxxflags_cmd = reduce(lambda cmd, flag: cmd + flag, ["cxxflags=%s " % flag for flag in cxxflags])

		toolset_cmd = "--toolset=%s" % toolset.boost_name()
		stage_cmd = "--stagedir=\"%s\"" % stage

		#bjam toolset stagedir address-model defs cxxflags libs options
		boost_cmd = "bjam %s %s %s %s %s %s %s" % (
			toolset_cmd, stage_cmd, address_model, defs_cmd, cxxflags_cmd, libs_cmd, options)
		report_info( 'Executing: %s' % boost_cmd )

		env = os.environ
		if not proj.customized_toolset_dir() is None:
			env = add_binpath(os.environ, [proj.customized_toolset_dir()])
		if subprocess.call(boost_cmd, env=env, stdout = sys.stdout) != 0:
			report_error("Boost build failed.")

		os.chdir(proj.source_root())
		report_info("Boost build done.")
		return True
	else:
		os.chdir(proj.source_root())
		report_error("Unsupported OS.")


def clean_llvm(proj):
	guarded_rmtree(proj.llvm_build())
	guarded_rmtree(proj.llvm_install())

def config_and_make_cmake_project(project_name, additional_params, source_dir, build_dir, install_dir, proj):
	if additional_params is None:
		conf_params = dict()
	else:
		conf_params = additional_params.copy()
	if not proj.toolset().short_compiler_name() == "vc":
		conf_params["CMAKE_BUILD_TYPE"] = ("STRING", proj.config_name())
	if not install_dir is None:
		conf_params["CMAKE_INSTALL_PREFIX"] = ("PATH", install_dir)
	
	params_cmd = reduce(lambda cmd, lib: cmd + lib, [' -D %s:%s="%s"' % (k, v[0], v[1]) for (k, v) in conf_params.items()])
	
	report_info("Configuring %s ..." % project_name)
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)
	if systems.current() == systems.win32:
		conf_cmd = batch_command(build_dir)
		if proj.customized_toolset_dir():
			conf_cmd.add_command('@set PATH=%%PATH%%;"%s"' % proj.customized_toolset_dir())
		conf_cmd.add_command('"%s" -G "%s" %s %s ' % (proj.cmake_exe(), proj.generator(), params_cmd, source_dir))
		conf_cmd.add_command('@if ERRORLEVEL 1 exit /B 1')
		if conf_cmd.execute() != 0:
			report_error("%s configure failed." % project_name)
	else:
		report_error("Unsupported OS.")
		
	report_info("Making %s on %s ..." % (project_name, proj.config_name()))
	if proj.toolset().short_compiler_name() == "vc":
		make_cmd = batch_command(build_dir)
		make_cmd.add_command('@call "%s"' % proj.env_setup_commands())
		make_cmd.add_command('@set VisualStudioVersion=%s' % proj.toolset().vs_version_string())
		make_cmd.add_command('@%s ALL_BUILD.%s /m /v:m /p:Configuration=%s'
			% (proj.maker_name(), proj.project_file_ext(), proj.config_name()))
		make_cmd.add_command('@if ERRORLEVEL 1 exit /B 1')
		if not install_dir is None:
			make_cmd.add_command(
				'@%s INSTALL.%s /m /v:m /p:Configuration=%s' % (proj.maker_name(), proj.project_file_ext(), proj.config_name()))
			make_cmd.add_command('@if ERRORLEVEL 1 exit /B 1')
		if make_cmd.execute() != 0:
			report_error("%s make failed." % project_name)
	elif proj.toolset().short_compiler_name() == "mgw":
		make_cmd = batch_command(build_dir)
		make_cmd.add_command('@%s' % proj.env_setup_commands())
		make_cmd.add_command('%s -j%d' % (proj.maker_name(), multiprocessing.cpu_count()))
		make_cmd.add_command('@if ERRORLEVEL 1 exit /B 1')
		if not install_dir is None:
			make_cmd.add_command('@%s install' % proj.maker_name())
			make_cmd.add_command('@if ERRORLEVEL 1 exit /B 1')
		if make_cmd.execute() != 0:
			report_error("%s make failed." % project_name)
	else:
		report_error("Unsupported toolset or OS.")
	
def config_and_make_llvm(proj):
	# Add definitions here
	defs = dict()
	defs["PYTHON_EXECUTABLE"] = ("PATH", sys.executable)
	defs["LLVM_BOOST_DIR"] = ("PATH", proj.boost_root())
	defs["LLVM_BOOST_STDINT"] = ("BOOL", "TRUE")
	config_and_make_cmake_project('LLVM', defs, proj.llvm_root(), proj.llvm_build(), proj.llvm_install(), proj)

def config_and_make_freeimage(proj):
	config_and_make_cmake_project('FreeImage', None, proj.freeimage_root(), proj.freeimage_build(), proj.freeimage_install(), proj)
	
def config_and_make_freetype(proj):
	config_and_make_cmake_project('FreeType', None, proj.freetype_root(), proj.freetype_build(), proj.freetype_install(), proj)
	
def clean_salvia(proj):
	guarded_rmtree(proj.salvia_build())
	guarded_rmtree(proj.salvia_lib())
	guarded_rmtree(proj.salvia_bin())

def config_and_make_salvia(proj):
	defs = dict()
	defs["SALVIA_BOOST_DIR"] = ("PATH", proj.boost_root())
	defs["PYTHON_EXECUTABLE"] = ("PATH", sys.executable)
	defs["SALVIA_BOOST_LIB_DIR"] = ("PATH", proj.boost_lib_dir() )
	if proj.toolset().short_compiler_name() == "vc":
		defs["SALVIA_FREEIMAGE_DIR"] = ( "PATH", proj.freeimage_install_in_msvc() )
		defs["SALVIA_LLVM_INSTALL_DIR"] = ("PATH", proj.llvm_install_in_msvc() )
		defs["SALVIA_FREETYPE_DIR"] = ( "PATH", proj.freetype_install_in_msvc() )
	else:
		defs["SALVIA_FREEIMAGE_DIR"] = ( "PATH", proj.freeimage_install() )
		defs["SALVIA_LLVM_INSTALL_DIR"] = ("PATH", proj.llvm_install() )
		defs["SALVIA_FREETYPE_DIR"] = ( "PATH", proj.freetype_install() )
	defs["SALVIA_BUILD_WITH_DIRECTX"] = ("BOOL", "TRUE" if proj.directx() else "FALSE")
	config_and_make_cmake_project('SALVIA', defs, proj.source_root(), proj.salvia_build(), None, proj)

def install_prebuild_binaries(proj):
	print( "Installing dependencies ..." )
	# Copy FreeImage
	#fi_bin_root = os.path.join( proj.source_root(), "3rd_party", "FreeImage", "bin" )
	#fi_dll = None
	#fi_bin_dir = os.path.join( fi_bin_root, proj.target_modifier(['platform']) )
	#fi_dll = os.path.join( fi_bin_dir, 'FreeImage.dll')
	#copy_newer( fi_dll, proj.salvia_bin() )

	# Copy boost
	files = os.listdir(proj.boost_lib_dir())
	need_copy = []
	if proj.os() == systems.win32:
		for f in files:
			f_basename = os.path.basename(f)
			name, ext = os.path.splitext(f)
			if not ext in ['.dll', '.so']:
				continue
			if ('-gd-' in name or '-d-' in name) != (proj.config_name() == "Debug"):
				continue
			if not proj.toolset().boost_lib_name() in name:
				continue
			need_copy.append(os.path.join(proj.boost_lib_dir(), f_basename))

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

def build(proj_props, cleanBuild):
	log_f = open("build.log", "w")
	atexit.register(close_log)

	proj = project(proj_props, os.getcwd())
	proj.print_props()
	proj.check()

	make_bjam(proj)
	if cleanBuild: clean_all(proj)
	make_boost(proj)

	config_and_make_freetype(proj)
	config_and_make_freeimage(proj)
	config_and_make_llvm(proj)
	config_and_make_salvia(proj)

	install_prebuild_binaries(proj)

if __name__ == "__main__":
	try:
		opts, args = getopt.getopt(sys.argv[1:], "c", ['clean'])
	except getopt.GetoptError:
		print("Option parsed error.")
		os.system("pause")
		sys.exit(1)

	if copy_newer("build_conf.tmpl", "proj.py"):
		print( "Project file was generated.\nPlease edit proj.py and run build_all.py again." )
		os.system('pause')
		sys.exit(1)

	report_info("Salvia building start.")
	# Load Project
	prj_props = __import__("proj")

	for opt, arg in opts:
		if opt == "-c" or opt == "--clean":
			proj = project(prj_props, os.getcwd())
			proj.print_props()
			clean_all(proj)
			os.system("pause")
			sys.exit(0)

	try:
		build(prj_props, False)
	except building_error as err:
		print("[E] " + err.message())
		print("[E] Salvia built failed.")
		os.system("pause")
		sys.exit(1)
	
	report_info("Salvia building done.")
	os.system("pause")
	