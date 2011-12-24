#!/usr/bin/env python

#-*- coding:utf-8 -#-

import os, sys, re, atexit
from functools	import *

from blibs			import *
from blibs.copy		import *
from blibs.env		import *
from blibs.util		import *
from blibs.project	import *

log_f = None
def close_log():
	global log_f
	if log_f != None:
		log_f.close()

def make_bjam( prj ):
	old_dir = os.curdir
	src = prj.boost_root()
	os.chdir( src )
	if prj.current_os() == systems.win32:
		if not os.path.exists( "bjam.exe" ):
			print( 'Building Bjam ...')
			os.system( "bootstrap.bat" )
		else:
			print( 'Bjam is existed.' )
	else:
		print("ERROR: Boost build doesn't support non-win32 for now.")
		os.chdir( old_dir )
		return False
	os.chdir( old_dir )
	return True

def make_boost( proj ):
	src = proj.boost_root()
	stage = proj.boost_stage()

	os.chdir( src )
	#Get boost build command
	if proj.current_os() == systems.win32:
		# Add configs
		libs = ["thread", "system", "filesystem", "date_time", "test", "wave", "program_options", "serialization"]
		address_model = 'address-model=%d' % proj.arch().bits()
		options = "--build-dir=./ link=shared runtime-link=shared threading=multi stage"
		toolset = proj.toolset()
		defs = []
		cxxflags = []
		if toolset == None:
			print("ERROR: Unspecified toolset was used.")
			return False
		if toolset.compiler_name == 'msvc':
			defs = ["_CRT_SECURE_NO_DEPRECATE", "_SCL_SECURE_NO_DEPRECATE"]
			cxxflags = ["-wd4819", "-wd4910"]

		#configs to cmd
		libs_cmd = reduce( lambda cmd, lib: cmd+lib, [ "--with-%s " % lib for lib in libs ] )
		defs_cmd = reduce( lambda cmd, dfn: cmd+dfn, [ "define=%s " % dfn for dfn in defs ] )
		cxxflags_cmd = reduce( lambda cmd, flag: cmd+flag, [ "cxxflags=%s " % flag for flag in cxxflags ] )

		toolset_cmd = "--toolset=%s" % toolset.boost_name()
		stage_cmd = "--stagedir=\"%s\"" % stage

		#bjam toolset stagedir address-model defs cxxflags libs options
		boost_cmd = "bjam %s %s %s %s %s %s %s" % (toolset_cmd, stage_cmd, address_model, defs_cmd, cxxflags_cmd, libs_cmd, options)
		print( 'Executing: %s' % boost_cmd )
		os.system(boost_cmd)
	else:
		return False

	os.chdir( proj.source_root() )
	return True

def config_llvm( proj ):
	# Add definitions here
	defs = {}
	defs["LLVM_BOOST_DIR"] = ("PATH", proj.boost_root())
	defs["LLVM_BOOST_STDINT"] = ("BOOL", "TRUE")
	defs["CMAKE_INSTALL_PREFIX"] = ("PATH", proj.llvm_install())
	defs_cmd = reduce( lambda cmd, lib: cmd+lib, [ "-D %s:%s=%s " % (k, v[0], v[1]) for (k, v) in defs.items() ] )
	
	print("WARNING: All directories referred by SALVIA *MUST NOT INCLUDE* space.")
	print("Configuring LLVM ...")
	llvm_cmd = 'cmake -G "%s" %s %s ' % (proj.generator(), defs_cmd, proj.llvm_root() )
	print( "-- Executing: %s" % llvm_cmd )
	
	if not os.path.exists( proj.llvm_build() ):
		os.makedirs( proj.llvm_build() )
	
	old_dir = os.curdir
	os.chdir( proj.llvm_build() )
	os.system( llvm_cmd )
	os.chdir( old_dir )
	pass
	
def make_llvm( proj ):
	#Write command to build.bat
	cmd = batch_command( proj.llvm_build() )
	cmd.add_command( '@call "%s"' % proj.env_setup_commands() )
	cmd.add_command( '@echo Building LLVM %s ...' % proj.config_name() )
	cmd.add_command( '@devenv.exe LLVM.sln /build %s' % proj.config_name() )
	cmd.add_command( '@echo Installing LLVM %s ...' % proj.config_name() )
	cmd.add_command( '@devenv.exe LLVM.sln /build %s /project Install' % proj.config_name() )
	cmd.execute()
	pass

def config_salvia( conf ):
	# Add definitions here
	defs = {}
	defs["SALVIA_BOOST_DIRECTORY"] = ("PATH", conf.boost_root())
	defs["SALVIA_BOOST_LIB_DIR"] = ("PATH", os.path.join( conf.boost_stage(), "lib" ) )
	defs["SALVIA_LLVM_INSTALL_PATH"] = ("PATH", conf.llvm_install())
	defs["SALVIA_BUILD_WITH_LLVM"] = ("BOOL", "TRUE")
	defs["SALVIA_ENABLE_SASL_REGRESSION_TEST"] = ("BOOL", "TRUE")
	defs["SALVIA_ENABLE_SASL_SEPERATED_TESTS"] = ("BOOL", "TRUE")
	
	defs_cmd = reduce( lambda cmd, lib: cmd+lib, [ "-D %s:%s=%s " % (k, v[0], v[1]) for (k, v) in defs.items() ] )
	
	print("WARNING: All directories referred by SALVIA *MUST NOT INCLUDE* space.")
	print("Configuring Salvia ...")
	salvia_cmd = 'cmake -G "%s" %s %s ' % (conf.generator(), defs_cmd, conf.source_root() )
	print( "-- Executing: %s" % salvia_cmd )
	
	if not os.path.exists( conf.salvia_build() ):
		os.makedirs( conf.salvia_build() )
	
	os.chdir( conf.salvia_build() )
	os.system( salvia_cmd )
	os.chdir( conf.source_root() )
	pass
	
def make_salvia( conf, build_cfgs):
	#Write command to build.bat
	cmd = batch_command( conf.salvia_build() )
	
	#cmd.add_command( '@echo off' )
	cmd.add_command( '@call "%s"' % conf.env_setup_commands() )
	for cfg in build_cfgs:
		cmd.add_command( '@echo Building SALVIA %s ...' % cfg )
		cmd.add_command( '@devenv.exe salvia.sln /build %s' % cfg )
	cmd.execute()
	pass

def install_prebuild_binaries( conf, build_cfgs ):
	print( "Installing dependencies ..." )
	# Copy FreeImage
	fi_bin_root = os.path.join( conf.source_root(), "3rd_party", "FreeImage", "bin" )
	fi_dll = None
	if conf.os() == systems.win32:
		if conf.arch() == arch.x86:
			fi_bin_dir = os.path.join( fi_bin_root, "win32" )
		elif conf.arch() == arch.x64:
			fi_bin_dir = os.path.join( fi_bin_root, "x64" )
		fi_dll = os.path.join( fi_bin_dir, 'FreeImage.dll')
	if fi_dll:
		for cfg in build_cfgs:
			copy_newer( fi_dll, conf.salvia_bin(cfg) )
		
	# Copy boost
	files = os.listdir( conf.boost_lib_dir() )
	need_copy = []
	for cfg in build_cfgs:
		if conf.os() == systems.win32:
			for f in files:
				f_basename = os.path.basename(f)
				name, ext = os.path.splitext(f)
				if( ext != ".dll" ): continue
				if ( '-gd' in name ) != (cfg == "Debug"): continue
				if ( not conf.toolset().boost_lib_name() in name ): continue
				need_copy.append( os.path.join( conf.boost_lib_dir(), f_basename ) )
	
		for f in need_copy:
			copy_newer( f, conf.salvia_bin(cfg) )
		
def clean_all():
	pass

if __name__ == "__main__":
	log_f = open("build.log", "w")
	atexit.register(close_log)

	
	if not os.path.exists( "proj.py" ):
		print( "Project file was generated.\nPlease edit proj.py and run build_all.py again." )
		copy_newer( "build_conf.tmpl", "proj.py" )
		sys.exit(1)
	copy_newer( "build_conf.tmpl", "proj.py" )

	# Load Project
	prj_props = __import__( "proj" )
	proj = project( prj_props )
	proj.print_props()

	# Support win32 only.
	if proj.current_os() != systems.win32:
		print("ERROR: Boost build doesn't support non-win32 for now.")
		sys.exit(1)

	print( 'Checking boost...')
	if not proj.boost_version():
		print('ERROR: Boost is not found. Please specify boost path in \'build_conf.py\'.')
		sys.exit(1)
	else:
		print('Boost is checked.')

	print( 'Checking bjam ...' )
	if not make_bjam( proj ):
		sys.exit(1)

	print( 'Building boost ...' )
	if not make_boost( proj ):
		sys.exit(1)

	config_llvm( proj )
	# make_llvm( conf, ['Debug'] )
	# config_salvia( conf )
	# make_salvia( conf, ['Debug'] )

	# install_prebuild_binaries( conf, ['Debug'] )
	
	# print( 'Build done.')
	os.system("pause")
	