#include <eflib/include/platform/dl_loader.h>

#ifdef EFLIB_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif

using boost::shared_ptr;

namespace eflib{

#ifdef EFLIB_WINDOWS

	class win_dl: public dynamic_lib{
	public:
		virtual void* get_function( std::string const& name ) const{
			if ( !mod ) return NULL;
			return static_cast<void*>( ::GetProcAddress( mod, name.c_str() ) );
		}
		virtual bool available() const {
			return mod != NULL;
		}

		HMODULE mod;
	};

	shared_ptr<dynamic_lib> dynamic_lib::load( std::string const& name ){
		HMODULE mod = ::LoadLibraryA( name.c_str() );

		win_dl* dynlib = new win_dl();
		dynlib->mod = mod;

		return shared_ptr<dynamic_lib>(dynlib);
	}

#else
#error "Have not support non-windows platform yet!"
#endif
}