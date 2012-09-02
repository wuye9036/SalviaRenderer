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
	defs_cmd = reduce( lambda cmd, lib: cmd+lib, [ ' -D %s:%s="%s"' % (k, v[0], v[1]) for (k, v) in defs.items() ] )
	
	print("Configuring LLVM ...")
	if not os.path.exists( proj.llvm_build() ):
		os.makedirs( proj.llvm_build() )
	llvm_cmd = batch_command( proj.llvm_build() )
	llvm_cmd.add_command( '"%s" -G "%s" %s %s ' % (proj.cmake_exe(), proj.generator(), defs_cmd, proj.llvm_root() ) )
	llvm_cmd.execute()
	pass
	
def make_llvm( proj ):
	#Write command to build.bat
	cmd = batch_command( proj.llvm_build() )
	cmd.add_command( '@call "%s"' % proj.env_setup_commands() )
	cmd.add_command( '@echo Building LLVM %s ...' % proj.config_name() )
	cmd.add_command( '@%s LLVM.sln /build %s /project ALL_BUILD' % (proj.maker_name(), proj.config_name()) )
	cmd.add_command( '@echo Installing LLVM %s ...' % proj.config_name() )
	cmd.add_command( '@%s LLVM.sln /build %s /project Install' % (proj.maker_name(), proj.config_name()) )
	cmd.execute()
	pass

def make_freetype( proj ):
	cmd = batch_command( proj.freetype_solution() )
	cmd.add_command( '@call "%s"' % proj.env_setup_commands() )
	cmd.add_command( '@echo Building FreeType2 %s ...' % proj.config_name() )
	cmd.add_command( '@%s freetype.sln /build %s /project freetype' % (proj.maker_name(), proj.msvc_config_name_with_platform()) )
	cmd.execute()

	cmd = batch_command( proj.source_root() )
	cmd.add_command( '@echo Installing FreeType2 %s ...' % proj.config_name() )
	cmd.add_command( '@mkdir "%s"' % proj.freetype_install() )
	cmd.add_command( '@copy /y "%s" "%s"' % ( os.path.join(proj.freetype_build(), "freetype.lib"), os.path.join(proj.freetype_install(), "freetype.lib") ) )
	cmd.execute()

def config_salvia( proj ):
	# Add definitions here
	defs = {}
	defs["SALVIA_BOOST_DIRECTORY"] = ("PATH", proj.boost_root())
	defs["SALVIA_BOOST_LIB_DIR"] = ("PATH", proj.boost_lib_dir() )
	defs["SALVIA_LLVM_INSTALL_PATH"] = ("PATH", proj.prebuilt_llvm() )
	defs["SALVIA_BUILD_WITH_LLVM"] = ("BOOL", "TRUE")
	defs["SALVIA_FREETYPE_LIB_DIR"] = ( "PATH", proj.freetype_install_in_msvc() )
	defs["SALVIA_BUILD_WITH_DIRECTX"] = ("BOOL", "TRUE" if proj.directx() else "FALSE")
	defs["SALVIA_ENABLE_SASL_REGRESSION_TEST"] = ("BOOL", "TRUE")
	defs["SALVIA_ENABLE_SASL_SEPERATED_TESTS"] = ("BOOL", "TRUE")
	
	defs_cmd = reduce( lambda cmd, lib: cmd+lib, [ '-D %s:%s="%s" ' % (k, v[0], v[1]) for (k, v) in defs.items() ] )
	
	if not os.path.exists( proj.salvia_build() ):
		os.makedirs( proj.salvia_build() )

	print("Configuring Salvia ...")
	salvia_cmd = batch_command( proj.salvia_build() )
	salvia_cmd.add_command( 'cmake -G "%s" %s %s ' % (proj.generator(), defs_cmd, proj.source_root()) )
	salvia_cmd.execute()
	
def make_salvia( proj ):
	cmd = batch_command( proj.salvia_build() )
	cmd.add_command( '@call "%s"' % proj.env_setup_commands() )
	cmd.add_command( '@echo Building SALVIA %s ...' % proj.config_name() )
	cmd.add_command( '@%s salvia.sln /build %s /project ALL_BUILD' % (proj.maker_name(), proj.config_name()) )
	cmd.execute()

def install_prebuild_binaries( proj ):
	print( "Installing dependencies ..." )
	# Copy FreeImage
	fi_bin_root = os.path.join( proj.source_root(), "3rd_party", "FreeImage", "bin" )
	fi_dll = None
	fi_bin_dir = os.path.join( fi_bin_root, proj.target_modifier(['platform']) )
	fi_dll = os.path.join( fi_bin_dir, 'FreeImage.dll')
	copy_newer( fi_dll, proj.salvia_bin() )
		
	# Copy boost
	files = os.listdir( proj.boost_lib_dir() )
	need_copy = []
	if proj.os() == systems.win32:
		for f in files:
			f_basename = os.path.basename(f)
			name, ext = os.path.splitext(f)
			if( ext != ".dll" ): continue
			if ( '-gd' in name ) != (proj.config_name() == "Debug"): continue
			if ( not proj.toolset().boost_lib_name() in name ): continue
			need_copy.append( os.path.join( proj.boost_lib_dir(), f_basename ) )
	
	for f in need_copy:
		copy_newer( f, proj.salvia_bin() )
		
def clean_all():
	pass

def build(proj_props):
	log_f = open("build.log", "w")
	atexit.register(close_log)
	
	proj = project(proj_props)
	proj.print_props()
	
	# Support win32 only.
	if proj.current_os() != systems.win32:
		print("ERROR: Boost build doesn't support non-win32 for now.")
		sys.exit(1)

	print( 'Checking boost...')
	if not proj.boost_version():
		print('ERROR: Boost is not found. Please specify boost path in \'proj.py\'.')
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
	make_llvm( proj )
	make_freetype(proj)
	config_salvia( proj )
	make_salvia( proj )

	install_prebuild_binaries( proj )
	
if __name__ == "__main__":
	if copy_newer( "build_conf.tmpl", "proj.py" ):
		print( "Project file was generated.\nPlease edit proj.py and run build_all.py again." )
		os.system('pause')
		sys.exit(1)

	# Load Project
	prj_props = __import__( "proj" )
	build(prj_props)
	
	# print( 'Build done.')
	os.system("pause")
	