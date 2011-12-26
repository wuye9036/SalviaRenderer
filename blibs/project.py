
from functools			import *
from blibs.env			import *
from blibs.boost_build	import *

class project:
	def __init__(self, props):
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
		
		# Initialize toolset
		env = os.environ
		default_toolset = props.toolset
		
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
	
	def print_props(self):
		print( '='*25 + ' Checking Project Properties ' + '='*25 )
		print( ' * Current Arch ............ %s' % str( self.current_arch() ) )
		print( ' * Current OS .............. %s' % str( self.current_os() ) )
		print( ' * Toolset ................. %s' % self.toolset().short_name() )
		print( ' * Env Script(win32 only) .. %s' % os.path.normpath( self.env_setup_commands() ) )
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
			if os.path.exists( os.path.join( self.builder_root_, 'Common7', 'IDE', 'VCExpress.exe' ) ):
				return 'VCExpress.exe'
			return 'devenv.exe'
		else:
			return 'make'
			
	def config_name(self):
		return self.config_
	
	def to_abs(self, path):
		if os.path.isabs( path ): return path
		else: return os.path.abspath( os.path.join( self.source_root(), path ) )
	
	def source_root(self):
		return os.path.dirname( sys.argv[0] )
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

	def salvia_build(self):
		return os.path.join( self.build_root(), "salvia_" + self.target_modifier(['platform', 'tool']) )
	def salvia_bin(self):
		return os.path.join( self.install_bin(), self.target_modifier(['platform']), self.target_modifier(['config']) )
		
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
