#ifndef SASL_SYNTAX_TREE_MAKE_TREE_H
#define SASL_SYNTAX_TREE_MAKE_TREE_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>

#include <sasl/enums/buildin_type_code.h>
#include <sasl/enums/literal_constant_types.h>
#include <sasl/include/syntax_tree/node_creation.h>
#include <eflib/include/boostext.h>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <string>

namespace sasl{
	namespace common{
		struct token_attr;
	}
}

BEGIN_NS_SASL_SYNTAX_TREE();

typedef boost::mpl::vector<
	bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t,
	float, double
> cpptypes;

extern literal_constant_types type_codes[];

template<typename T>
struct is_sasl_buildin_type : public boost::mpl::not_< 
	boost::is_same<
		typename boost::mpl::find<cpptypes, T>::type,
		typename boost::mpl::end<cpptypes>::type 
	>
>::type{};

template <typename T>
buildin_type_code cpptype_to_typecode( 
	EFLIB_DISABLE_IF_COND( is_sasl_buildin_type<T>, 0 )
	)
{
	return type_codes[boost::mpl::find<cpptypes, T>::type::pos];
}

boost::shared_ptr<::sasl::common::token_attr> null_token();

struct buildin_type;
struct constant_expression;
struct declaration;
struct declaration_statement;
struct expression;
struct function_type;
struct initializer;
struct type_specifier;
struct variable_declaration;

class tree_combinator
{
	virtual tree_combinator& dvar( const std::string& var_name ){
		syntax_error();
		return *this;
	}
	virtual tree_combinator& dstruct( const std::string& struct_name ){
		syntax_error();
		return *this;
	}
	virtual tree_combinator& dfunction( const std::string& func_name ){
		syntax_error();
		return *this;
	}
	virtual tree_combinator& dtypedef( const std::string& alias ){
		syntax_error();
		return *this;
	}

	virtual tree_combinator& end(){ return parent; }
	
	template <typename T>
	tree_combinator& end( boost::shared_ptr<T>& result )
	{
		result = boost::shared_polymorphic_cast<T>( curnode );
	}
protected:
	tree_combinator( const tree_combinator& );
	tree_combinator& operator = ( const tree_combinator& );

	tree_combinator( tree_combinator& parent ): parent( parent ){}

private:
	void syntax_error(){
		assert( !"Fuck!" );
	}

protected:
	boost::shared_ptr<node> curnode;
	tree_combinator& parent;
};

class dprog_combinator: public tree_combinator{
public:
	dprog_combinator();

	virtual tree_combinator& dvar( const std::string& var_name );
	virtual tree_combinator& dstruct( const std::string& struct_name );
	virtual tree_combinator& dfunction( const std::string& func_name );
	virtual tree_combinator& dtypedef( const std::string& alias );

	virtual tree_combinator& end();
private:
	boost::shared_ptr<program> typed_node;
};

class dvar_combinator: public tree_combinator{
public:
	dvar_combinator( const std::string& var_name );

	// virtual tree_combinator&
};
END_NS_SASL_SYNTAX_TREE()
#endif