import os, sys, platform

class arch:
	def __init__(self, tag):
		self.__tag = tag

	@staticmethod
	def from_machine( machine ):
		if machine == 'x86' or machine == 'i386':
			return arch.x86
		if machine == 'x64' or machine == 'AMD64':
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
		return arch.from_machine( platform.machine() )

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
		if self == systems.win32:
			return 'nt'
		if self == systems.linux:
			return 'linux'
		return 'unknown'

systems.unknown = systems(0)
systems.win32 = systems(1)
systems.linux = systems(2)

class toolset:
	def __init__(self, ide_name, compiler_name, major_version, minor_version, patch_version):
		self.ide_name = ide_name
		self.compiler_name = compiler_name
		self.major_ver = major_version
		self.minor_ver = minor_version
		self.patch_ver = patch_version

	def boost_name(self):
		return "%s-%d.%d" % (self.compiler_name, self.major_ver, self.minor_ver)
		
	def short_name(self):
		ret = "%s%d" % ( self.compiler_name, self.major_ver )
		need_patch_ver = ( self.patch_ver and self.patch_ver != 0 )
		need_minor_ver = need_patch_ver or ( self.minor_ver and self.minor_ver != 0 )
		if need_minor_ver: ret += str(self.minor_ver)
		if need_patch_ver: ret += str(self.patch_ver)
		return ret

	def boost_lib_name(self):
		ret = "%s%d%d" % ( self.short_compiler_name(), self.major_ver, self.minor_ver )
		need_patch_ver = ( self.patch_ver and self.patch_ver != 0 )
		if need_patch_ver:
			ret += str(self.patch_ver)
		return ret
		
	def short_compiler_name(self):
		if self.compiler_name == "msvc":
			return "vc"
		elif self.compiler_name == "mingw":
			return "mgw"
		else:
			return None
		