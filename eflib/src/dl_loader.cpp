#include <eflib/include/platform/dl_loader.h>

#ifdef EFLIB_WINDOWS
#	if !defined NOMINMAX
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif

#ifdef EFLIB_LINUX
#	include <dlfcn.h>
#endif

using boost::shared_ptr;

namespace eflib{

#ifdef EFLIB_WINDOWS

	class win_dl: public dynamic_lib{
	public:
		win_dl(std::string const& name)
		{
			mod = ::LoadLibraryA( name.c_str() );
			void (*loaded_hook)() = NULL;
			dynamic_lib::get_function(loaded_hook, "_eflib_dynlib_loaded");
			if(loaded_hook)
			{
				loaded_hook();
			}
		}

		~win_dl()
		{
			void (*unloading_hook)() = NULL;
			dynamic_lib::get_function(unloading_hook, "_eflib_dynlib_unloading");
			if(unloading_hook)
			{
				unloading_hook();
			}

			::FreeLibrary(mod);
			mod = (HMODULE)0;
		}

		virtual void* get_function( std::string const& name ) const
		{
			if ( !mod ) return NULL;
			return (void*)( ::GetProcAddress( mod, name.c_str() ) );
		}

		virtual bool available() const
		{
			return mod != NULL;
		}

		HMODULE mod;
	};
#elif defined(EFLIB_LINUX)
    class linux_dl: public dynamic_lib
    {
	public:
		linux_dl(std::string const& name)
		{
			mod = ::dlopen(name.c_str(), 0);
			void (*loaded_hook)() = NULL;
			dynamic_lib::get_function(loaded_hook, "_eflib_dynlib_loaded");
			if(loaded_hook)
			{
				loaded_hook();
			}
		}

		~linux_dl()
		{
			void (*unloading_hook)() = NULL;
			dynamic_lib::get_function(unloading_hook, "_eflib_dynlib_unloading");
			if(unloading_hook)
			{
				unloading_hook();
			}

			dlclose(mod);
			mod = nullptr;
		}

		virtual void* get_function( std::string const& name ) const
		{
			if ( !mod ) return NULL;
			return (void*)( dlsym( mod, name.c_str() ) );
		}

		virtual bool available() const
		{
			return mod != NULL;
		}

		void* mod;
	};
#endif

	shared_ptr<dynamic_lib> dynamic_lib::load( std::string const& name )
	{
#if defined(EFLIB_WINDOWS)
		win_dl* dynlib = new win_dl(name);
#elif defined(EFLIB_LINUX)
		linux_dl* dynlib = new linux_dl(name);
#else
#	error "Platform is not supported."
#endif
		return shared_ptr<dynamic_lib>(dynlib);
	}
}
