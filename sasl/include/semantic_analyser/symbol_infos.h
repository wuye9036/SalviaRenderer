#ifndef SASL_SEMANTIC_ANALYSER_SYMBOL_INFOS_H
#define SASL_SEMANTIC_ANALYSER_SYMBOL_INFOS_H

#include <sasl/include/semantic_analyser/semantic_analyser_forward.h>
#include <sasl/include/syntax_tree/symbol_info.h>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>

namespace sasl {
	namespace syntax_tree{
		struct constant;
		struct type_specifier;

		class symbol_info;
	}
}

BEGIN_NS_SASL_SEMANTIC_ANALYSER();

using sasl::syntax_tree::constant;
using sasl::syntax_tree::symbol_info;
using sasl::syntax_tree::type_specifier;

class value_symbol_info: public symbol_info{
public:
	typedef sasl::syntax_tree::symbol_info base_type;
	value_symbol_info();
	value_symbol_info( const constant& c );
	template <typename T> T value(){
		return boost::get<T>(val);
	}

private:
	boost::variant< unsigned long, long, double, std::string, bool, char > val;
};

class value_type_symbol_info: public symbol_info{
public:
	typedef symbol_info base_type;
	value_type_symbol_info();
	value_type_symbol_info( boost::shared_ptr<type_specifier> spec );

	boost::shared_ptr<type_specifier> value_type();
private:
	boost::shared_ptr<type_specifier> valtype;
};

END_NS_SASL_SEMANTIC_ANALYSER();

#endif