#ifndef SASL_ENUMS_DEFAULT_HASHER_H
#define SASL_ENUMS_DEFAULT_HASHER_H

#include <sasl/enums/enum_base.h>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/functional/hash.hpp>

struct enum_hasher{
	template <typename EnumT> size_t val( const EnumT& e ) const{
		return (size_t)e.val_;
	}
	template <typename EnumT> size_t operator() ( const EnumT& e ) const{
		return val(e);
	}
};

namespace boost{
	template <typename D, typename S> ::std::size_t hash_value( enum_base<D, S> const& e ){
		return enum_hasher().val(e);
	}
}
#endif
