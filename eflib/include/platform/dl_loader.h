#ifndef EFLIB_PLATFORM_DL_LOADER_H
#define EFLIB_PLATFORM_DL_LOADER_H

#include <eflib/include/string/string.h>

namespace eflib{
	class dynamic_lib{
	public:
		static boost::shared_ptr<dynamic_library> load( std::_tstring const& name );
		
		template <typename FunctionT>
		void get_function( FunctionT& fn, std::_tstring const& name ) const;
		
	private:
		virtual void* get_function( std::_tstring const& name ) const;
	};
}

#endif