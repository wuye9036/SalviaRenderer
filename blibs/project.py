import systems

class project:
	def __init__(self, boost_root, build_root, install_root, arch, toolset, config):
		self.boost_ver_ = boost_version(boost_root)
		if self.boost_ver_:
			self.boost_root_ = boost_root
		else:
			self.boost_root_ = None
		self.build_root_ = build_root
		self.install_root_ = install_root
		self.arch_ = arch.from_machine(arch)
		self.os_ = systems.current()
		self.config_ = config
		
		# Initialize toolset
		env = os.environ
		default_toolset = toolset
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
		hint_dict["platform"] = str(self.os()) + str(self.arch())
		hint_dict["config"] = self.config_
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
	
	def config_name(self):
		return self.config_
	
	def install_root(self):
		return self.install_root_	
	def install_bin(self):
		return os.path.join( self.source_root(), build_conf.install_root, "bin" )
	def install_lib(self):
		return os.path.join( self.source_root(), build_conf.install_root, "lib" )
		
	# Boost builds.
	def boost_root(self):
		return self.boost_root_
	def boost_version(self):
		return self.boost_ver_
	def boost_stage(self):
		return os.path.join( self.install_lib(), 'boost_' % target_modifier(['platform']) )
	def boost_lib_dir(self):
		return os.path.join( self.boost_stage(), "lib" )
		
	def llvm_fullname(self):
		return "llvm_" + str( self.arch() ) + "_" + str( self.toolset().short_name() )
	def llvm_root(self):
		return os.path.join( self.source_root(), "3rd_party", "llvm" )
	def llvm_build(self):
		return os.path.join( self.build_root(), self.llvm_fullname() )
	def llvm_install(self):
		return os.path.join( self.install_lib(), self.llvm_fullname()+'_' )

	def salvia_fullname(self):
		return "salvia_" + self.target_modifier(['arch', 'tool'])
	def salvia_build(self):
		return os.path.join( self.build_root(), self.salvia_fullname() )
	def salvia_lib(self):
		pass
	def salvia_bin(self, cfg):
		return os.path.join( self.install_bin(), self.target_modifier(['arch']), cfg )
	def salvia_install(self):
		pass
		
	def builder(self):
		return self.builder_
		
	# Sources
	def source_root(self):
		return os.path.dirname( sys.argv[0] )
	
	# Builds
	def build_root(self):
		return self.build_root_
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
