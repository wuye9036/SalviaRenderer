#!/usr/bin/env python
#-*- coding:utf-8 -#-

import os, sys, re, atexit, getopt
from functools	import *

from blibs			import *
from blibs.copy		import *
from blibs.env		import *
from blibs.util		import *
from blibs.project	import *

def guarded_rmtree(path):
	if os.path.isdir(path):
		shutil.rmtree(path)

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

def clean_boost(proj):
	src = proj.boost_root()
	stage = proj.boost_stage()

	os.chdir(src)
	if proj.current_os() == systems.win32:
		boost_cmd = "bjam --clean"
		print( 'Executing: %s' % boost_cmd )
		os.system(boost_cmd)
	else:
		return False

	os.chdir( proj.source_root() )
	guarded_rmtree( proj.boost_stage() )

def make_boost(proj):
	src = proj.boost_root()
	stage = proj.boost_stage()

	os.chdir( src )
	#Get boost build command
	if proj.current_os() == systems.win32:
		# Add configs
		libs = ["atomic", "chrono", "thread", "system", "filesystem", "date_time", "test", "wave", "program_options", "serialization"]
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

def clean_llvm(proj):
	guarded_rmtree( proj.llvm_build() )
	guarded_rmtree( proj.llvm_install() )
	
def config_llvm(proj):
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
	
def make_llvm(proj):
	#Write command to build.bat
	cmd = batch_command( proj.llvm_build() )
	cmd.add_command( '@call "%s"' % proj.env_setup_commands() )
	cmd.add_command( '@echo Building LLVM %s ...' % proj.config_name() )
	cmd.add_command( '@%s LLVM.sln /build %s /project ALL_BUILD' % (proj.maker_name(), proj.config_name()) )
	cmd.add_command( '@echo Installing LLVM %s ...' % proj.config_name() )
	cmd.add_command( '@%s LLVM.sln /build %s /project Install' % (proj.maker_name(), proj.config_name()) )
	cmd.execute()
	pass

def clean_freetype(proj):
	guarded_rmtree( proj.freetype_build() )
	objs_root = os.path.join(proj.freetype_root(), "objs")
	for dir in os.listdir(objs_root):
		full_path = os.path.join(objs_root, dir)
		if os.path.isdir(full_path):
			guarded_rmtree(full_path)
	guarded_rmtree(proj.freetype_install())
	
def make_freetype(proj):
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

def clean_salvia(proj):
	guarded_rmtree( proj.salvia_build() )
	guarded_rmtree( proj.salvia_lib() )
	guarded_rmtree( proj.salvia_bin() )
	
def config_salvia(proj):
	# Add definitions here
	defs = {}
	defs["SALVIA_BOOST_DIRECTORY"] = ("PATH", proj.boost_root())
	defs["SALVIA_BOOST_LIB_DIR"] = ("PATH", proj.boost_lib_dir() )
	defs["SALVIA_LLVM_INSTALL_PATH"] = ("PATH", proj.prebuilt_llvm() )
	defs["SALVIA_FREETYPE_LIB_DIR"] = ( "PATH", proj.freetype_install_in_msvc() )
	defs["SALVIA_BUILD_WITH_DIRECTX"] = ("BOOL", "TRUE" if proj.directx() else "FALSE")

	defs_cmd = reduce( lambda cmd, lib: cmd+lib, [ '-D %s:%s="%s" ' % (k, v[0], v[1]) for (k, v) in defs.items() ] )
	
	if not os.path.exists( proj.salvia_build() ):
		os.makedirs( proj.salvia_build() )

	print("Configuring Salvia ...")
	salvia_cmd = batch_command( proj.salvia_build() )
	salvia_cmd.add_command( '@"%s" -G "%s" %s %s ' % (proj.cmake_exe(), proj.generator(), defs_cmd, proj.source_root()) )
	salvia_cmd.execute()
	
def make_salvia(proj):
	cmd = batch_command( proj.salvia_build() )
	cmd.add_command( '@call "%s"' % proj.env_setup_commands() )
	cmd.add_command( '@echo Building SALVIA %s ...' % proj.config_name() )
	cmd.add_command( '@%s salvia.sln /build %s /project ALL_BUILD' % (proj.maker_name(), proj.config_name()) )
	cmd.execute()

def install_prebuild_binaries(proj):
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
	
def clean_all(proj):
	print('clean Boost ...')
	clean_boost(proj)
	print('clean FreeType2 ...')
	clean_freetype(proj)
	print('clean LLVM ...')
	clean_llvm(proj)
	print('clean SALVIA ...')
	clean_salvia(proj)
	pass

def build(proj_props, cleanBuild):
	log_f = open("build.log", "w")
	atexit.register(close_log)
	
	proj = project( proj_props, os.getcwd() )
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
	if not make_bjam(proj):
		sys.exit(1)

	if cleanBuild:
		clean_all(proj)
		
	print( 'Building boost ...' )
	if not make_boost(proj):
		sys.exit(1)

	config_llvm(proj)
	make_llvm(proj)
	make_freetype(proj)
	config_salvia(proj)
	make_salvia(proj)

	install_prebuild_binaries(proj)
	
if __name__ == "__main__":
	try:
		opts, args = getopt.getopt(sys.argv[1:], "c", ['clean'])
	except getopt.GetoptError:
		print("Option parsed error.")
		os.system("pause")
		sys.exit(1)

	if copy_newer( "build_conf.tmpl", "proj.py" ):
		print( "Project file was generated.\nPlease edit proj.py and run build_all.py again." )
		os.system('pause')
		sys.exit(1)

	# Load Project
	prj_props = __import__( "proj" )
	
	for opt, arg in opts:
		if opt == "-c" or opt == "--clean":
			proj = project( prj_props, os.getcwd() )
			proj.print_props()
			clean_all(proj)
			os.system("pause")
			sys.exit(0)

	build(prj_props, False)
	os.system("pause")
	