#ifndef SASL_SEMANTIC_NAME_MANGLER_H
#define SASL_SEMANTIC_NAME_MANGLER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/enums/default_hasher.h>
#include <sasl/enums/buildin_type_code.h>
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

BEGIN_NS_SASL_SEMANTIC();

class name_mangler{
public:
	name_mangler();
	std::string mangle( boost::shared_ptr<::sasl::syntax_tree::function_type> node );
private:
	boost::unordered_map< buildin_type_code, std::string > btc_decorators;
	boost::unordered_map< type_qualifiers, std::string > qual_decorators;
	std::string mangled_name;
	
	void mangle_basic_name( const std::string& );
	void mangle_type( boost::shared_ptr<::sasl::syntax_tree::type_specifier> );
};

END_NS_SASL_SEMANTIC();
#endif