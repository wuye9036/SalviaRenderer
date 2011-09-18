#!/usr/bin/env python

#-*- coding:utf-8 -#-

from build_scripts import *
import os, sys, platform, re
import build_conf

class arch:
	def __init__(self, tag):
		self.__tag = tag

	@staticmethod
	def from_machine( machine ):
		if machine == 'i386':
			return arch.x86
		if machine == 'AMD64':
			return arch.x64
		return arch.unknown
	
	def bits(self):
		if self == arch.x86:
			return 32
		if self == arch.x64:
			return 64
		return 0

	@staticmethod
	def current():
		return from_machine( platform.machine() )

	def __str__(self):
		if self == arch.x86:
			return 'x86'
		if self == arch.x64:
			return 'x64'
		return 'unknown'

arch.unknown = arch(0)
arch.x86 = arch(1)
arch.x64 = arch(2)

class systems:
	def __init__(self, tag):
		self.__tag = tag

	@staticmethod
	def current():
		if platform.system() == 'Windows':
			return systems.win32
		if platform.system() == 'Linux':
			return systems.linux
		return systems.unknown

	def __str__(self):
		if self == win32:
			return 'win32'
		if self == linux:
			return 'linux'
		return 'unknown'

systems.unknown = arch(0)
systems.win32 = arch(1)
systems.linux = arch(2)

class toolset:
	def __init__(self, ide_name, compiler_name, major_version, minor_version, patch_version):
		self.ide_name = ide_name
		self.compiler_name = compiler_name
		self.major_ver = major_version
		self.minor_ver = minor_version
		self.patch_ver = patch_version

	def boost_toolset(self):
		return "%s-%d.%d" % (self.compiler_name, self.major_ver, self.minor_ver)

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
				print('ERROR: Cannot found any valid compiler on your computer.')

		if default_toolset == "msvc-10.0":
			self.toolset_ = toolset('vs', 'msvc', 10, 0, None)
		elif default_toolset == 'msvc-9.0':
			self.toolset_ = toolset('vs', 'msvc', 9, 0, None)
		elif default_toolset == 'msvc-8.0':
			self.toolset_ = toolset('vs', 'msvc', 8, 0, None)
		else:
			print('ERROR: Unsupported toolset name: %s' % default_toolset)
		
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

	# Boost builds.
	def boost_root(self):
		return self.boost_root_
	def boost_version(self):
		return self.boost_ver_
	
	# Sources
	def source_root(self):
		return os.path.dirname( sys.argv[0] )
	
	# Builds
	def build_path(self):
		return self.build_path_

	# Targets
	def install_path(self):
		return self.install_path_

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
		print("Boost build doesn't support non-win32 for now.")
		return False
	os.chdir( build_config.instance().source_root() )
	return True

def make_boost( src, stage_dir ):
	os.chdir( src )
	#Get boost build command
	if build_config.instance().current_os() == systems.win32:
		libs = ["thread", "system", "filesystem", "date_time", "test"]
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

		libs_cmd = reduce( lambda cmd, lib: cmd+lib, [ "--with-%s " % lib for lib in libs ] )
		defs_cmd = reduce( lambda cmd, dfn: cmd+dfn, [ "define=%s " % dfn for dfn in defs ] )
		cxxflags_cmd = reduce( lambda cmd, flag: cmd+flag, [ "cxxflags=%s " % flag for flag in cxxflags ] )

		toolset_cmd = "--toolset=%s" % toolset.boost_toolset()
		stage_cmd = "--stagedir=\"%s\"" % stage_dir

		#bjam toolset stagedir address-model defs cxxflags libs options
		boost_cmd = "bjam %s %s %s %s %s %s %s" % (toolset_cmd, stage_cmd, address_model, defs_cmd, cxxflags_cmd, libs_cmd, options)
		print( 'Executing: %s' % boost_cmd )
		os.system(boost_cmd)
	else:
		return False

	os.chdir( build_config.instance().source_root() )
	return True
	
def make_llvm( src, dest ):
	pass

if __name__ == "__main__":
	# Load configuration
	conf = build_config.instance()

	# Support win32 only.
	if conf.current_os() != systems.win32:
		print("ERROR: Boost build doesn't support non-win32 for now.")
		sys.exit(1)

	print( 'Compiling SALVIA...' )

	print( 'Configuring compiler...' )

	print( 'Compiling externals...')

	print( 'Finding boost...')
	if not conf.boost_version():
		print('ERROR: Boost is not found. Please edit its path in \'build_conf.py\'.')
		sys.exit(1)
	print( 'Boost %s is found.' % conf.boost_version() )

	print( 'Building bjam ...' )
	if not make_bjam( conf.boost_root() ):
		sys.exit(1)

	print( 'Building boost ...' )
	boost_stage_root = conf.build_path()
	if not os.path.isabs(boost_stage_root):
		boost_stage_root = os.path.join( conf.source_root(), conf.build_path() )
	boost_stage = os.path.join( boost_stage_root, 'boost-%s' % conf.boost_version(), str(conf.arch()) )
	if not make_boost( conf.boost_root(), boost_stage ):
		sys.exit(1)

	print( 'Configuring LLVM ...' )
	print( 'Building LLVM...' )

	print( 'Configuring SALVIA ...' )
	print( 'Building SALVIA ...' )

	print( 'Build done.')
	os.system("pause")
	