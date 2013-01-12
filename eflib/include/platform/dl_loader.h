#ifndef EFLIB_PLATFORM_DL_LOADER_H
#define EFLIB_PLATFORM_DL_LOADER_H

#include <eflib/include/string/string.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/identity.hpp>
#include <eflib/include/platform/boost_end.h>

namespace eflib{
	class dynamic_lib{
	public:
		static boost::shared_ptr<dynamic_lib> load( std::string const& name );

		template <typename PFnT>
		bool get_function( PFnT& fn, std::string const& name ) const
		{
			void* pfn = get_function(name);
			if( pfn ) {
				fn = static_cast<PFnT>(pfn);
				return true;
			}
			fn = NULL;
			return false;
		}
		
		virtual bool available() const = 0;
		virtual ~dynamic_lib() {}

	private:
		virtual void* get_function( std::string const& name ) const = 0;
	};

}

#define EFLIB_IMPORT_DLL_FUNCTION( fn_type, fn_name, dy_lib, sym_name )	\
	boost::mpl::identity<fn_type>::type fn_name = NULL;	\
	(dy_lib)->get_function( (fn_name), #sym_name );

#endif