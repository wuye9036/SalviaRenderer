#ifndef SASL_SEMANTIC_ANALYSER_SYMBOL_INFOS_H
#define SASL_SEMANTIC_ANALYSER_SYMBOL_INFOS_H

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/include/semantic/symbol_info.h>
#include <sasl/enums/type_types.h>
#include <sasl/enums/literal_constant_types.h>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <boost/weak_ptr.hpp>

namespace sasl {
	namespace syntax_tree{
		struct constant;
		struct type_specifier;
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::constant;
using ::sasl::syntax_tree::type_specifier;
using ::sasl::syntax_tree::node;

class value_symbol_info: public symbol_info{
public:
	typedef symbol_info base_type;
	value_symbol_info();
	value_symbol_info( const std::string& vallit, literal_constant_types ctype );
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

class type_symbol_info: public symbol_info{
public:
	friend class symbol;
	typedef symbol_info base_type;

	type_symbol_info( type_types ttype );
	type_types type_type() const;

	boost::shared_ptr<node> full_type() const;
	void full_type( boost::shared_ptr<node> ftnode );
private:
	type_symbol_info();
	type_types ttype;
	boost::weak_ptr<node> type_node;
};

class variable_symbol_info: public symbol_info{
public:
	friend class symbol;
	typedef symbol_info base_type;
	variable_symbol_info( bool is_local );
	bool is_local() const;
private:
	variable_symbol_info();
	bool isloc;
};
END_NS_SASL_SEMANTIC();

#endif