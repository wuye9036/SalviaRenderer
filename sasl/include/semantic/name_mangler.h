#ifndef SASL_SEMANTIC_NAME_MANGLER_H
#define SASL_SEMANTIC_NAME_MANGLER_H

#include <eflib/include/platform/config.h>

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/enums/default_hasher.h>
#include <sasl/enums/builtin_types.h>
#include <sasl/enums/type_qualifiers.h>
#include <sasl/enums/storage_mode.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <string>

namespace sasl{
	namespace syntax_tree{
		struct type_specifier;
		struct array_type;
		struct struct_type;
		struct function_type;
	}
}

struct operators;

BEGIN_NS_SASL_SEMANTIC();

std::string mangle( boost::shared_ptr< ::sasl::syntax_tree::function_type > node );
std::string operator_name( const operators& );

END_NS_SASL_SEMANTIC();
#endif