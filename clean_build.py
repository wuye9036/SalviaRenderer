import build_all

class build_properties:
	def __init__(self, props):
		self.__dict__ = dict( self.__dict__.items() + props.items() )
		pass
		
if __name__ == "__main__":
	props_ntx86_msvc10_debug = build_properties( {
			'boost_root' 	: 'E:/Programming/Library/boost_1_44_0',
			'build_root' 	: '../salvia_build',
			'install_root'	: '../salvia_ntx86_msvc10/',
			'arch'			: 'x86',
			'toolset'		: 'msvc-10.0',
			'config'		: 'Debug',
			'cmake'			: 'cmake'
		} )
	#Clean target first
	build_all.build_all(props_ntx86_msvc10_debug)
	pass
