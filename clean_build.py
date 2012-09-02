import build_all

class build_properties:
	def __init__(self, props):
		self.__dict__ = dict( self.__dict__.items() + props.items() )
		pass
		
if __name__ == "__main__":
	props = build_properties( {
			'boost_root' 	: 'D:/Programming/boost_1_48_0',
			'build_root' 	: '.\\build',
			'install_root'	: './',
			'arch'			: 'x86',
			'toolset'		: 'msvc-10.0',
			'config'		: 'Debug',
			'cmake'			: 'cmake'
		} )
	build_all.build_all(props)
	pass
