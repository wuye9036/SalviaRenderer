#ifndef SASL_SEMANTIC_ANALYSER_SEMANTIC_INFOS_IMP_H
#define SASL_SEMANTIC_ANALYSER_SEMANTIC_INFOS_IMP_H

#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/enums/enums_helper.h>

BEGIN_NS_SASL_SEMANTIC();

template <typename T> T const_value_si::value() const {

    if ( sasl_ehelper::is_integer( value_type()  ) ) {
        if ( sasl_ehelper::is_unsigned( value_type() ) ) {
            return (T)boost::get<uint64_t>(val);
        }
        return (T)boost::get<int64_t>(val);
    }

    if( sasl_ehelper::is_real( value_type() ) ) {
        return (T)boost::get<double>(val);
    }

    if ( value_type() == builtin_types::_boolean ) {
        return (T)boost::get<bool>(val);
    }

    return T();

}

template <typename T> void const_value_si::value( T val ){
	this->val = val;
	type_info()->value_typecode = builtin_types::none;
}
END_NS_SASL_SEMANTIC();

#endif
