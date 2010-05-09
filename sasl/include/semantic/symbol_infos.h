#ifndef SASL_SEMANTIC_ANALYSER_SYMBOL_INFOS_H
#define SASL_SEMANTIC_ANALYSER_SYMBOL_INFOS_H

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/include/semantic/symbol_info.h>
#include <sasl/enums/type_types.h>
#include <sasl/enums/literal_constant_types.h>
#include <sasl/enums/buildin_type_code.h>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <boost/weak_ptr.hpp>

namespace sasl {
	namespace syntax_tree{
		struct type_specifier;
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::type_specifier;
using ::sasl::syntax_tree::node;

class const_value_symbol_info: public symbol_info{
public:
	typedef symbol_info base_type;
	const_value_symbol_info();

	void constant_value_literal( const std::string& litstr, literal_constant_types lctype);

	template <typename T> T value() const{
		return boost::get<T>(val);
	}
	template <typename T> void value( T val ){
		this->val = val;
	}

	buildin_type_code value_type() const;
	void value_type( buildin_type_code vtype );

private:
	boost::variant< unsigned long, long, double, std::string, bool, char > val;
	buildin_type_code valtype;
};

class type_symbol_info: public symbol_info{
public:
	friend class symbol;
	typedef symbol_info base_type;

	type_types type_type() const;
	void type_type( type_types ttype );

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

	bool is_local() const;
	void is_local( bool isloc );
private:
	variable_symbol_info();
	bool isloc;
};
END_NS_SASL_SEMANTIC();

#endif