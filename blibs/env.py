import os, sys, platform, subprocess

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

arch.unknown = arch('arch.unknown')
arch.x86 = arch('arch.x86')
arch.x64 = arch('arch.x86-64')

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

systems.unknown = systems('system.unknown')
systems.win32 = systems('system.win32')
systems.linux = systems('system.linux')

class toolset:
	def __init__(self, ide_name, compiler_name, major_version, minor_version, patch_version):
		self.ide_name = ide_name
		self.compiler_name = compiler_name
		self.major_ver = major_version
		self.minor_ver = minor_version
		self.patch_ver = patch_version

	def support_x64(self):
		if self.compiler_name == "mingw":
			return False
		return True

	def boost_name(self):
		if self.compiler_name == "msvc":
			return "%s-%d.%d" % (self.compiler_name, self.major_ver, self.minor_ver)
		elif self.compiler_name in ["mingw", "mingw64", "gcc"]:
			return "gcc"

	def short_name(self):
		ret = "%s%d" % ( self.compiler_name, self.major_ver )
		need_patch_ver = ( self.patch_ver and self.patch_ver != 0 )
		need_minor_ver = need_patch_ver or ( self.minor_ver and self.minor_ver != 0 )
		if need_minor_ver: ret += str(self.minor_ver)
		if need_patch_ver: ret += str(self.patch_ver)
		return ret

	def boost_lib_name(self):
		ret = "%s%d%d" % ( self.short_compiler_name(), self.major_ver, self.minor_ver )
		return ret
		
	def short_compiler_name(self):
		if self.compiler_name == "msvc":
			return "vc"
		elif self.compiler_name == "mingw" or self.compiler_name == "mingw64":
			return "mgw"
		elif self.compiler_name == "gcc":
			return "gcc"
		else:
			return None
			
	def vs_version_string(self):
		if self.compiler_name == "msvc":
			return '%d.%d' % (self.major_ver, self.minor_ver)
		return None

def detect_cmake(candidate_cmake_executable):
	cmake = candidate_cmake_executable \
		if not candidate_cmake_executable is None else "cmake"
	try:
		subprocess.call([cmake, "--version"])
		return cmake
	except:
		return None

def detect_gcc(gcc_dir, min_major_ver, min_minor_ver):
	gcc_executables = []
	if gcc_dir is not None:
		gcc_executables.append( os.path.join(gcc_dir, "g++") )
	if systems.current() == systems.win32:
		try:
			gcc_paths = subprocess.check_output(["@where", "g++"])
			gcc_executables += gcc_paths.split("\r\n")
		except:
			pass
		gcc_executables.append(os.path.join("C:/MinGW/bin", "g++"))

	for gcc_executable in gcc_executables:
		try:
			version_str = subprocess.check_output([gcc_executable, "-dumpversion"])
			version_digits = [int(s) for s in version_str.split('.')]
			while len(version_digits) < 3:
				version_digits.append(None)

			if version_digits[0] < min_major_ver:
				continue
			elif version_digits[0] == min_major_ver:
				if version_digits[1] < min_minor_ver:
					continue

			compiler_name = None
			machine_name = subprocess.check_output([gcc_executable, "-dumpmachine"]).strip()
			if machine_name == "x86_64-w64-mingw32":
				compiler_name = "mingw64"
			elif machine_name == "mingw32":
				compiler_name = "mingw"
			else:
				compiler_name = "gcc"
			gcc_toolset = toolset( None, compiler_name,
				version_digits[0], version_digits[1], version_digits[2]
			)
			return gcc_toolset, gcc_dir
		except:
			pass
	return None, None

def add_binpath(env, dirs):
	seperator = ';' if systems.current() == systems.win32 else ':'
	new_env = env.copy()
	for dir in dirs:
		new_env['PATH'] = env['PATH'] + seperator + dir
	return new_env

def windows_kit_dirs():
	if systems.current() == systems.win32:
		kits = None
		close_key = None

		try:
			import _winreg
			winkit_key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, r"SOFTWARE\Microsoft\Windows Kits\Installed Roots")
			if winkit_key is None:
				return None
			def CloseKey():
				_winreg.CloseKey(winkit_key)
			close_key = CloseKey
			kit_names = ["KitsRoot", "KitsRoot81"]
			kits = [ str(kit)
					for kit in map(lambda key_name: _winreg.QueryValueEx(winkit_key, key_name)[0], kit_names)
					if kit is not None]

		except ImportError as e:
			return None
			
		finally:
			if close_key:
				close_key()
			return kits

	return None
