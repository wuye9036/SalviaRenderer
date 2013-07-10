
from functools			import *
from blibs.env			import *
from blibs.boost_build	import *

class project:
	def __init__(self, props, cwd):
		self.boost_ver_ = boost_version(props.boost_root)
		if self.boost_ver_:
			self.boost_root_ = props.boost_root
		else:
			self.boost_root_ = None
		self.build_root_ = props.build_root
		self.install_root_ = props.install_root
		self.arch_ = arch.from_machine(props.arch)
		self.os_ = systems.current()
		self.config_ = props.config
		self.cwd_ = cwd
		
		# Initialize toolset
		self.cmake_ = props.cmake
		env = os.environ
		default_toolset = props.toolset
		
		self.directx_ = ("DXSDK_DIR" in env)
			
		if default_toolset == None or default_toolset == "":
			if "VS110COMNTOOLS" in env:
				default_toolset = "msvc-11.0"
			if "VS100COMNTOOLS" in env:
				default_toolset = "msvc-10.0"
			elif os.path.exists("C:\\Mingw\\bin\\gcc.exe"):
				default_toolset = "mingw"
			else:
				print('ERROR: Cannot found any valid compiler on your computer. Please set toolset in "build_conf.py" ')
				
		if default_toolset == "msvc-11.0":
			self.toolset_ = toolset('vs', 'msvc', 11, 0, None)
			self.builder_root_ = os.path.join( env['VS110COMNTOOLS'], "../../" )
		if default_toolset == "msvc-10.0":
			self.toolset_ = toolset('vs', 'msvc', 10, 0, None)
			self.builder_root_ = os.path.join( env['VS100COMNTOOLS'], "../../" )
		else:
			print('ERROR: Unsupported toolset name: %s' % default_toolset)
	
	def print_props(self):
		print( '='*25 + ' Checking Project Properties ' + '='*25 )
		print( ' * Current Arch ............ %s' % str( self.current_arch() ) )
		print( ' * Current OS .............. %s' % str( self.current_os() ) )
		print( ' * Toolset ................. %s' % self.toolset().short_name() )
		print( ' * Env Script(win32 only) .. %s' % os.path.normpath( self.env_setup_commands() ) )
		print( ' * CMake Executable ........ %s' % self.cmake_exe() )
		print( ' * CMake Generator ......... %s' % self.generator() )
		print( ' * Make tool ............... %s' % self.maker_name() )
		print('')
		print( ' * Target .................. %s' % self.target_modifier(['platform', 'tool', 'config']) )
		print( ' * Source .................. %s' % self.source_root() )
		print( ' * Build ................... %s' % self.build_root() )
		print( ' * Install ................. %s' % self.install_root() )
		print( ' * Install Binaries ........ %s' % self.install_bin() )
		print( ' * Install Libraries ....... %s' % self.install_lib() )
		print('')
		print( ' * Boost ................... %s' % self.boost_root() )
		print( ' * Boost Version ........... %s' % self.boost_version() )
		print( ' * Boost Stage ............. %s' % self.boost_stage() )
		print( ' * LLVM .................... %s' % self.llvm_root() )
		print( ' * LLVM Build .............. %s' % self.llvm_build() )
		print( ' * LLVM Install ............ %s' % self.llvm_install() )
		print( ' * FreeType2 ............... %s' % self.freetype_root() )
		print( ' * FreeType2 Solution....... %s' % self.freetype_solution() )
		print( ' * FreeType2 Build ......... %s' % self.freetype_build() )
		print( ' * FreeType2 Install ....... %s' % self.freetype_install() )
		print('')
		print( ' * SALVIA Build ............ %s' % self.salvia_build() )
		print( ' * SALVIA Binaries ......... %s' % self.salvia_bin() )
		print( "========== Project Properties ==========" )
		pass
		
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
		hint_dict["platform"] = str(self.os()) + str(self.arch())
		hint_dict["config"] = self.config_
		return reduce( lambda ret,s: ret+"_"+s, [hint_dict[hnt] for hnt in hints] )
		
	def env_setup_commands(self):
		if self.os() == systems.win32 and self.toolset().compiler_name == 'msvc':
			base_dir = os.path.join( self.builder_root_, "vc", "bin" )
			if self.current_arch() == arch.x86 or self.vc_express():
				return os.path.join( base_dir, "vcvars32.bat" )
			if self.current_arch() == arch.x64:
				return os.path.join( base_dir, "x86_amd64", "vcvarsx86_amd64.bat" )
		else:
			print("Unrecognized OS.")
	
	def vc_express(self):
		return self.maker_name() == "VCExpress.exe"
		
	def maker_name(self):
		if self.toolset().compiler_name == 'msvc':
			return 'MSBuild'
		else:
			return 'make'
	
	def directx(self):
		return self.directx_
	
	def config_name(self):
		return self.config_
	
	def msvc_platform_name(self):
		if self.arch() == arch.x86:
			platform_name_in_msvc = "Win32"
		elif self.arch() == arch.x64:
			platform_name_in_msvc = "x64"
		return platform_name_in_msvc
		
	def project_file_ext(self):
		if self.toolset_.short_compiler_name() == "vc":
			if self.toolset_.major_ver >= 10:
				return "vcxproj"
			return "vcproj"
		else:
			assert False
			return None
		
	def msvc_config_name_with_platform(self):
		return '"' + self.config_ + "|" + self.msvc_platform_name() + '"'
	
	def to_abs(self, path):
		if os.path.isabs( path ): return path
		else: return os.path.abspath( os.path.join( self.source_root(), path ) )
	
	def source_root(self):
		source_root_path = os.path.dirname( sys.argv[0] )
		if os.path.isabs(source_root_path): return source_root_path
		else: return os.path.abspath( os.path.join(self.cwd_, source_root_path) )
	def build_root(self):
		return self.to_abs( self.build_root_ )
	def install_root(self):
		return self.to_abs( self.install_root_ )
		
	def install_bin(self):
		return os.path.join( self.install_root(), "bin" )
	def install_lib(self):
		return os.path.join( self.install_root(), "lib" )
		
	# Boost builds.
	def boost_root(self):
		return self.to_abs( self.boost_root_ )
	def boost_version(self):
		return self.boost_ver_
	def boost_stage(self):
		return os.path.join( self.install_lib(), 'boost_%s' % self.target_modifier(['platform']) )
	def boost_lib_dir(self):
		return os.path.join( self.boost_stage(), "lib" )
		
	def prebuilt_llvm(self):
		if self.os() != systems.win32:
			return llvm_install
		elif self.toolset().compiler_name == 'msvc':
			return os.path.join( self.install_lib(), "llvm_" + self.target_modifier(['platform', 'tool']) + '_$(ConfigurationName)' )
	def llvm_root(self):
		return os.path.join( self.source_root(), "3rd_party", "llvm" )
	def llvm_build(self):
		return os.path.join( self.build_root(), "llvm_" + self.target_modifier(['platform', 'tool']) )
	def llvm_install(self):
		return os.path.join( self.install_lib(), "llvm_" + self.target_modifier(['platform', 'tool', 'config']) )

	def freetype_root(self):
		return os.path.join( self.source_root(), "3rd_party", "freetype2")
	def freetype_solution(self):
		if self.current_os() == systems.win32:
			if self.toolset().short_name() == "msvc10":
				return os.path.join( self.freetype_root(), "builds", "win32", "vc2010" )
			elif self.toolset().short_name() == "msvc11":
				return os.path.join( self.freetype_root(), "builds", "win32", "vc2012" )
		return None
	def freetype_build(self):
		return os.path.join( self.freetype_root(), "libs", self.target_modifier(['platform', 'tool', 'config']) )
	def freetype_install(self):
		return os.path.join( self.install_lib(), "freetype_" + self.target_modifier(['platform', 'tool', 'config']) )
	def freetype_install_in_msvc(self):
		return os.path.join( self.install_lib(), "freetype_" + self.target_modifier(['platform', 'tool']) + '_$(ConfigurationName)' )
	def salvia_build(self):
		return os.path.join( self.build_root(), "salvia_" + self.target_modifier(['platform', 'tool']) )
	def salvia_lib(self):
		return os.path.join( self.install_lib(), self.target_modifier(['platform', 'tool']), self.target_modifier(['config']) )
	def salvia_bin(self):
		return os.path.join( self.install_bin(), self.target_modifier(['platform', 'tool']), self.target_modifier(['config']) )
		
	def generator(self):
		if self.arch() == arch.x86:
			if self.toolset().short_name() == 'msvc11':
				return "Visual Studio 11"
			if self.toolset().short_name() == 'msvc10':
				return "Visual Studio 10"
			else:
				print( "Unknown generator.")
				return None
		elif self.arch() == arch.x64:
			if self.toolset().short_name() == 'msvc11':
				return "Visual Studio 11 Win64"
			if self.toolset().short_name() == 'msvc10':
				return "Visual Studio 10 Win64"
			else:
				print ("Unknown generator.")
				return None
		else:
			print( "Unknown generator.")
			return None
