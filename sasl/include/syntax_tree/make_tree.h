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

struct function_tag{};
struct constant_tag{};

boost::shared_ptr<::sasl::common::token_attr> make_tree( const ::std::string& lit );
boost::shared_ptr<buildin_type> make_tree( const buildin_type_code btc );

template <typename T> boost::shared_ptr<variable_declaration> make_tree( 
	boost::shared_ptr<T> typespec,
	boost::shared_ptr<::sasl::common::token_attr> ident,
	EFLIB_ENABLE_IF_PRED2( is_base_of, type_specifier, T, 0 )
	)
{
	boost::shared_ptr<variable_declaration> ret = create_node<variable_declaration>( null_token() );
	ret->name = ident;
	ret->type_info = typespec;
	return ret;
}

template <typename T, typename U > boost::shared_ptr<variable_declaration> make_tree( 
	boost::shared_ptr<T> typespec,
	boost::shared_ptr<::sasl::common::token_attr> ident,
	boost::shared_ptr<U> init,
	EFLIB_ENABLE_IF_PRED2( is_base_of, type_specifier, T, 0 ),
	EFLIB_ENABLE_IF_PRED2( is_base_of, initializer, U, 1 )
	)
{
	boost::shared_ptr<variable_declaration> ret = create_node<variable_declaration>( null_token() );
	ret->name = ident;
	ret->type_info = typespec;
	ret->init = init;
	return ret;
}

template< typename ReturnT >
boost::shared_ptr<function_type> make_tree(
	boost::shared_ptr<ReturnT> ret_type, boost::shared_ptr<::sasl::common::token_attr> name, function_tag, 
	EFLIB_ENABLE_IF_PRED2(is_base_of, type_specifier, ReturnT, 0)
	)
{
	boost::shared_ptr<function_type> ret = create_node< function_type >( null_token() );
	ret->retval_type = boost::shared_polymorphic_cast<type_specifier>(ret_type);
	ret->name = name;
	return ret;
}

template <typename T>
boost::shared_ptr<declaration_statement> make_tree( boost::shared_ptr<T> decl, EFLIB_ENABLE_IF_PRED2( is_base_of, declaration, T, 0) ){
	boost::shared_ptr<declaration_statement> ret = create_node<declaration_statement>( null_token() );
	ret->decl = decl;
	return ret;
}

boost::shared_ptr<constant_expression> make_tree( literal_constant_types btc, const ::std::string& val);

template <typename T>
boost::shared_ptr<constant_expression> make_tree( T val, EFLIB_ENABLE_IF_COND( is_sasl_buildin_type<T>, 0 ) ){
	return make_tree(
		cpptype_to_typecode<T>(), boost::lexical_cast<string>(T)
	);
}

//template <typename T, typename U> boost::shared_ptr<binary_expression> make_node(
//	boost::shared_ptr<T> lexpr, operators op, boost::shared_ptr<U> rexpr,
//	EFLIB_ENABLE_IF_PRED2( is_base_of, expression, T, 0 ),
//	EFLIB_ENABLE_IF_PRED2( is_base_of, expression, U, 1 )
//	)
//{
//}
//
//template <typename T>
//boost::shared_ptr<declaration_statement> make_node(
//	boost::shared_ptr<T> decl,
//	EFLIB_ENABLE_IF_IS_BASE_OF(T, declaration)
//	)
//{
//}

END_NS_SASL_SYNTAX_TREE()
#endif