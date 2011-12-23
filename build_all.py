#!/usr/bin/env python

#-*- coding:utf-8 -#-

import os, sys, re, build_conf, atexit
from functools	import *

from blibs		import *
from blibs.copy	import *
from blibs.env	import *
from blibs.util	import *

log_f = None
def close_log():
	global log_f
	if log_f != None:
		log_f.close()
		
class build_config:
	__instance = None

	@staticmethod
	def instance():
		if not build_config.__instance:
			build_config.__instance = build_config()
		return build_config.__instance
		
	def __init__(self):
		self.boost_ver_ = boost_version(build_conf.boost_root)
		if self.boost_ver_:
			self.boost_root_ = build_conf.boost_root
		else:
			self.boost_root_ = None
		self.build_path_ = build_conf.build_path
		self.install_path_ = build_conf.install_path
		self.arch_ = arch.from_machine(build_conf.arch)
		self.os_ = systems.current()
		
		# Initialize toolset
		env = os.environ
		default_toolset = build_conf.toolset
		self.builder_ = None
		
		if default_toolset == None or default_toolset == "":
			if "VS100COMNTOOLS" in env:
				default_toolset = "msvc-10.0"
			elif "VS90COMNTOOLS" in env:
				default_toolset = "msvc-9.0"
			elif "VS80COMNTOOLS" in env:
				default_toolset = "msvc-8.0"
			elif os.path.exists("C:\\Mingw\\bin\\gcc.exe"):
				default_toolset = "mingw"
			else:
				print('ERROR: Cannot found any valid compiler on your computer. Please set toolset in "build_conf.py" ')
				
		if default_toolset == "msvc-10.0":
			self.toolset_ = toolset('vs', 'msvc', 10, 0, None)
			self.builder_root_ = os.path.join( env['VS100COMNTOOLS'], "../../" )
		elif default_toolset == 'msvc-9.0':
			self.toolset_ = toolset('vs', 'msvc', 9, 0, None)
			self.builder_root_ = os.path.join( env['VS90COMNTOOLS'], "../../" )
		elif default_toolset == 'msvc-8.0':
			self.toolset_ = toolset('vs', 'msvc', 8, 0, None)
			self.builder_root_ = os.path.join( env['VS80COMNTOOLS'], "../../" )
		else:
			print('ERROR: Unsupported toolset name: %s' % default_toolset)
		
		if build_conf.cmake == "" or build_conf.cmake == None or os.path.exists(build_conf.cmake):
			print('ERROR: Cannot find CMake executable. Please specify it in "build_conf.py"')
			self.cmake_ = None
		self.cmake_ = build_conf.cmake
		
	# Platform
	def arch(self):
		return self.arch_
	def os(self):
		return self.os_
		
	def current_arch(self):
		return arch.current()
	def current_os(self):
		return systems.current()

	def toolset(self):
		return self.toolset_
	def cmake_exe(self):
		return self.cmake_
	def target_modifier(self, hints):
		hint_dict = {}
		hint_dict["os"] = str( self.os() )
		hint_dict["arch"] = str( self.arch() )
		hint_dict["tool"] = str( self.toolset().short_name() )
		return reduce( lambda ret,s: ret+"_"+s, [hint_dict[hnt] for hnt in hints] )
		
	def env_setup_commands(self):
		base_dir = os.path.join( self.builder_root_, "vc/bin" )
		if self.os() == systems.win32:
			if self.current_arch() == arch.x86:
				return os.path.join( base_dir, "vcvars32.bat" )
			if self.current_arch() == arch.x64:
				return os.path.join( base_dir, "x86_amd64", "vcvarsx86_amd64.bat" )
		else:
			print("Unrecognized OS.")
	
	def install_path(self):
		return self.install_path_	
	def install_bin(self):
		return os.path.join( self.source_root(), build_conf.install_path, "bin" )
	def install_lib(self):
		return os.path.join( self.source_root(), build_conf.install_path, "lib" )
		
	# Boost builds.
	def boost_root(self):
		return self.boost_root_
	def boost_version(self):
		return self.boost_ver_
	def boost_stage(self):
		return os.path.join( self.install_lib(), 'boost-%s' % self.boost_version(), str(self.arch()) )
	def boost_lib_dir(self):
		return 
		
	def llvm_fullname(self):
		return "llvm_" + str( self.arch() ) + "_" + str( self.toolset().short_name() )
	def llvm_root(self):
		return os.path.join( self.source_root(), "3rd_party", "src", "llvm" )
	def llvm_build(self):
		return os.path.join( self.build_path(), self.llvm_fullname() )
	def llvm_install(self):
		return os.path.join( self.install_lib(), self.llvm_fullname() )

	def salvia_fullname(self):
		return "salvia_" + self.target_modifier(['arch', 'tool'])
	def salvia_build(self):
		return os.path.join( self.build_path(), self.salvia_fullname() )
	def salvia_lib(self):
		pass
	def salvia_bin(self):
		pass
	def salvia_install(self):
		pass
		
	def builder(self):
		return self.builder_
		
	# Sources
	def source_root(self):
		return os.path.dirname( sys.argv[0] )
	
	# Builds
	def build_path(self):
		return self.build_path_
	def generator(self):
		if self.arch() == arch.x86:
			if self.toolset().short_name() == 'msvc10':
				return "Visual Studio 10"
			else:
				print( "Unknown generator.")
				return None
		elif self.arch() == arch.x64:
			if self.toolset().short_name() == 'msvc10':
				return "Visual Studio 10 Win64"
			return None
		else:
			print( "Unknown generator.")
			return None
class filters:
	default_filter = lambda src, dest : True
	
class version_object:
	def __init__(self, version_int):
		self.version_int = version_int
		self.major = version_int / 100000
		self.minor = version_int / 100 % 1000
		self.patch = version_int % 100
		self.version_str = '%d.%d.%d' % (self.major, self.minor, self.patch)

	def __str__(self):
		return self.version_str

	def __cmp__(self, other):
		return self.version_int - other.version_int
	
def boost_version( boost_root ):
	"""
	Get boost version. 
	If it didn't include boost or unknown versioned boost,
	it will return None.
	"""
	version_hpp = os.path.join( boost_root, 'boost', 'version.hpp' )
	f = open(version_hpp)
	if f == None:
		return None
	version_lines = f.readlines()
	f.close()
	for line in version_lines:
		matched = re.match('\s*#\s*define\s+BOOST_VERSION\s+(?P<version>\d+)\s*', line )
		if matched and matched.groupdict() != None:
			if 'version' in matched.groupdict():
				return version_object( int(matched.group('version')) )
	return None

def make_bjam( src ):
	os.chdir( src )
	if build_config.instance().current_os() == systems.win32:
		if not os.path.exists( "bjam.exe" ):
			os.system( "bootstrap.bat" )
	else:
		print("ERROR: Boost build doesn't support non-win32 for now.")
		return False
	os.chdir( build_config.instance().source_root() )
	return True

def make_boost( src, stage_dir ):
	os.chdir( src )
	#Get boost build command
	if build_config.instance().current_os() == systems.win32:
		# Add configs
		libs = ["thread", "system", "filesystem", "date_time", "test", "wave"]
		address_model = 'address-model=%d' % build_config.instance().arch().bits()
		options = "--build-dir=./ link=shared runtime-link=shared threading=multi stage"
		toolset = build_config.instance().toolset()
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
		stage_cmd = "--stagedir=\"%s\"" % stage_dir

		#bjam toolset stagedir address-model defs cxxflags libs options
		boost_cmd = "bjam %s %s %s %s %s %s %s" % (toolset_cmd, stage_cmd, address_model, defs_cmd, cxxflags_cmd, libs_cmd, options)
		print( 'Executing: %s' % boost_cmd )
		os.system(boost_cmd)
	else:
		return False

	os.chdir( build_config.instance().source_root() )
	return True

def config_llvm( conf ):
	# Add definitions here
	defs = {}
	defs["LLVM_BOOST_DIR"] = ("PATH", conf.boost_root())
	defs["LLVM_BOOST_STDINT"] = ("BOOL", "TRUE")
	defs["CMAKE_INSTALL_PREFIX"] = ("PATH", conf.llvm_install())
	defs_cmd = reduce( lambda cmd, lib: cmd+lib, [ "-D %s:%s=%s " % (k, v[0], v[1]) for (k, v) in defs.items() ] )
	
	print("WARNING: All directories referred by SALVIA *MUST NOT INCLUDE* space.")
	print("Configuring LLVM ...")
	llvm_cmd = 'cmake -G "%s" %s %s ' % (conf.generator(), defs_cmd, conf.llvm_root() )
	print( "-- Executing: %s" % llvm_cmd )
	
	if not os.path.exists( conf.llvm_build() ):
		os.makedirs( conf.llvm_build() )
	
	os.chdir( conf.llvm_build() )
	os.system( llvm_cmd )
	os.chdir( conf.source_root() )
	pass
	
# configs := [ config, ... ]
# config  := 'Debug' | 'Release' | 'MinSizeRel' | 'RelWithDebInfo'
def make_llvm( conf, configs ):
	#Write command to build.bat
	cmd = batch_command( conf.llvm_build() )
	
	#cmd.add_command( '@echo off' )
	cmd.add_command( '@call "%s"' % conf.env_setup_commands() )
	for cfg in configs:
		cmd.add_command( '@echo Building LLVM %s ...' % cfg )
		cmd.add_command( '@devenv.exe LLVM.sln /build %s' % cfg )
		cmd.add_command( '@echo Installing LLVM %s ...' % cfg )
		cmd.add_command( '@devenv.exe LLVM.sln /build %s /project Install' % cfg )
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
	defs["SALVIA_ENABLE_SASL_SEPERATED_TESTS"] = ("BOOL", "FALSE")
	
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

def clean_all():
	pass

if __name__ == "__main__":
	log_f = open("build.log", "w")
	atexit.register(close_log)
	
	# Load configuration
	conf = build_config.instance()

	# Support win32 only.
	if conf.current_os() != systems.win32:
		print("ERROR: Boost build doesn't support non-win32 for now.")
		sys.exit(1)

	print( 'Configuring compiler...' )

	print( 'Compiling externals...')

	print( 'Finding boost...')
	if not conf.boost_version():
		print('ERROR: Boost is not found. Please specify boost path in \'build_conf.py\'.')
		sys.exit(1)
	print( 'Boost %s is found.' % conf.boost_version() )

	print( 'Building bjam ...' )
	if not make_bjam( conf.boost_root() ):
		sys.exit(1)

	print( 'Building boost ...' )
	boost_stage_root = conf.build_path()
	if not os.path.isabs(boost_stage_root):
		boost_stage_root = os.path.join( conf.install_lib() )
	boost_stage = os.path.join( boost_stage_root, 'boost-%s' % conf.boost_version(), str(conf.arch()) )
	#if not make_boost( conf.boost_root(), boost_stage ):
	#	sys.exit(1)

	print( "Finding CMake..." )
	if not conf.cmake_exe():
		print('ERROR: CMake is not found.')
		sys.exit(1)
	print( "CMake is found." )
	
	print( 'Configuring LLVM ...' )
	# config_llvm( conf )
	# make_llvm( conf, ['Debug'] )
	config_salvia( conf )
	make_salvia( conf, ['Debug'] )

	print( 'Build done.')
	os.system("pause")
	