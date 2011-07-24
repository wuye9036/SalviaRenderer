#ifndef SASL_SYNTAX_TREE_VISITOR
#define SASL_SYNTAX_TREE_VISITOR

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/preprocessor/cat.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SASL_SYNTAX_TREE();

struct unary_expression;
struct cast_expression;
struct expression_list;
struct cond_expression;
struct index_expression;
struct call_expression;
struct member_expression;
struct variable_expression;
struct identifier;
struct constant_expression;
struct binary_expression;

struct initializer;
struct expression_initializer;
struct member_initializer;
struct declaration;
struct declarator;
struct variable_declaration;
struct type_definition;
struct tynode;
struct builtin_type;
struct array_type;
struct struct_type;
struct alias_type;
struct parameter;
struct function_type;

struct statement;
struct declaration_statement;
struct if_statement;
struct while_statement;
struct dowhile_statement;
struct for_statement;
struct case_label;
struct ident_label;
struct switch_statement;
struct compound_statement;
struct expression_statement;
struct jump_statement;

struct program;

// Macros for making define visitor shortly.

// For expand SASL_VISIT_DEF_UNIMPL_ and SASL_VISIT_DEF
// #define SASL_VISITOR_TYPE_NAME
//

// It will be expand as a function as following:
//	void visit ( ... ){
//		EFLIB_ASSERT_UNIMPLEMENTED0( "XXX::visit was not implemented yet." );
//	}
#define SASL_VISIT_DEF_UNIMPL( node_type_name )	\
	SASL_SPECIFIC_VISIT_DEF_UNIMPL( visit, node_type_name )
#define SASL_SPECIFIC_VISIT_DEF_UNIMPL( fn, node_type_name ) \
	void SASL_VISITOR_TYPE_NAME::fn( ::sasl::syntax_tree::node_type_name &, ::boost::any* ){ \
		EFLIB_ASSERT_UNIMPLEMENTED0( \
			( ::std::string( BOOST_PP_STRINGIZE( SASL_VISITOR_TYPE_NAME::node_type_name ) ) \
			+ ::std::string( " was not implemented yet." ) ).c_str()	\
		); \
	}

#define SASL_VISIT_DEF( node_type_name )	\
	SASL_SPECIFIC_VISIT_DEF( visit, node_type_name )
#define SASL_SPECIFIC_VISIT_DEF( fn, node_type_name ) \
	void SASL_VISITOR_TYPE_NAME::fn ( ::sasl::syntax_tree::node_type_name& v, ::boost::any* data )

#define SASL_VISIT_DCL( node_type_name )	\
	SASL_SPECIFIC_VISIT_DCL( visit, node_type_name )
#define SASL_SPECIFIC_VISIT_DCL( fn, node_type_name ) \
	virtual void fn ( ::sasl::syntax_tree::node_type_name& v, ::boost::any* data = NULL )

#define SASL_VISIT_INLINE_DEF_UNIMPL( node_type_name )	\
	SASL_SPECIFIC_VISIT_INLINE_DEF_UNIMPL( visit, node_type_name )
#define SASL_SPECIFIC_VISIT_INLINE_DEF_UNIMPL( fn, node_type_name )	\
	virtual void fn ( ::sasl::syntax_tree::node_type_name &, ::boost::any* = NULL ){	\
		EFLIB_ASSERT_UNIMPLEMENTED0( \
			( ::std::string( BOOST_PP_STRINGIZE( node_type_name ) ) \
			+ ::std::string( " was not implemented yet." ) ).c_str()	\
		); \
	}

class syntax_tree_visitor{
public:
	// expression
	SASL_VISIT_DCL( unary_expression ) = 0;
	SASL_VISIT_DCL( cast_expression ) = 0;
	SASL_VISIT_DCL( binary_expression ) = 0;
	SASL_VISIT_DCL( expression_list ) = 0;
	SASL_VISIT_DCL( cond_expression ) = 0;
	SASL_VISIT_DCL( index_expression ) = 0;
	SASL_VISIT_DCL( call_expression ) = 0;
	SASL_VISIT_DCL( member_expression ) = 0;
	SASL_VISIT_DCL( constant_expression ) = 0;
	SASL_VISIT_DCL( variable_expression ) = 0;

	// declaration & type specifier
	SASL_VISIT_DCL( initializer ) = 0;
	SASL_VISIT_DCL( expression_initializer ) = 0;
	SASL_VISIT_DCL( member_initializer ) = 0;
	SASL_VISIT_DCL( declaration ) = 0;
	SASL_VISIT_DCL( declarator ) = 0;
	SASL_VISIT_DCL( variable_declaration ) = 0;
	SASL_VISIT_DCL( type_definition ) = 0;
	SASL_VISIT_DCL( tynode ) = 0;
	SASL_VISIT_DCL( builtin_type ) = 0;
	SASL_VISIT_DCL( array_type ) = 0;
	SASL_VISIT_DCL( struct_type ) = 0;
	SASL_VISIT_DCL( alias_type ) = 0;
	SASL_VISIT_DCL( parameter ) = 0;
	SASL_VISIT_DCL( function_type ) = 0;

	// statement
	SASL_VISIT_DCL( statement ) = 0;
	SASL_VISIT_DCL( declaration_statement ) = 0;
	SASL_VISIT_DCL( if_statement ) = 0;
	SASL_VISIT_DCL( while_statement ) = 0;
	SASL_VISIT_DCL( dowhile_statement ) = 0;
	SASL_VISIT_DCL( for_statement ) = 0;
	SASL_VISIT_DCL( case_label ) = 0;
	SASL_VISIT_DCL( ident_label ) = 0;
	SASL_VISIT_DCL( switch_statement ) = 0;
	SASL_VISIT_DCL( compound_statement ) = 0;
	SASL_VISIT_DCL( expression_statement ) = 0;
	SASL_VISIT_DCL( jump_statement ) = 0;

	// program
	SASL_VISIT_DCL( program ) = 0;
};

END_NS_SASL_SYNTAX_TREE();

#endif
