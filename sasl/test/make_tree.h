#ifndef SASL_TEST_MAKE_TREE_H
#define SASL_TEST_MAKE_TREE_H

#include <sasl/enums/buildin_type_code.h>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/preprocessor/cat.hpp>
#include <string>

namespace sasl{
	namespace common{
		struct token_attr;
	}
	namespace syntax_tree{
		struct buildin_type;
	}
}

#define SASL_ENABLE_IF_IS_BASE_OF( derived, base ) \
	typename ::boost::enable_if< ::boost::is_base_of<derived, base> >::type* BOOST_PP_CAT(dummy, derived) = 0

boost::shared_ptr<::sasl::common::token_attr> null_token();
boost::shared_ptr<::sasl::common::token_attr> make_tree( const ::std::string& lit );
boost::shared_ptr<::sasl::syntax_tree::buildin_type> make_tree( const buildin_type_code btc );
//
//template <typename T> boost::shared_ptr<variable_declaration> make_node( 
//	boost::shared_ptr<T> typespec,
//	const ::std::string& ident,
//	SASL_ENABLE_IF_IS_BASE_OF( T, sasl::syntax_tree::type_specifier)
//	)
//{
//}
//
//template <typename T, typename U> boost::shared_ptr<binary_expression> make_node(
//	boost::shared_ptr<T> lexpr, operators op, boost::shared_ptr<U> rexpr,
//	SASL_ENABLE_IF_IS_BASE_OF( T, expression ),
//	SASL_ENABLE_IF_IS_BASE_OF( U, expression )
//	)
//{
//}
//
//template <typename T>
//boost::shared_ptr<declaration_statement> make_node(
//	boost::shared_ptr<T> decl,
//	SASL_ENABLE_IF_IS_BASE_OF(T, declaration)
//	)
//{
//}
#endif