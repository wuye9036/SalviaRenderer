#ifndef SASL_SYNTAX_TREE_MAKE_TREE_H
#define SASL_SYNTAX_TREE_MAKE_TREE_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>

#include <sasl/enums/buildin_type_code.h>
#include <sasl/enums/literal_constant_types.h>
#include <sasl/enums/type_qualifiers.h>
#include <sasl/include/syntax_tree/node_creation.h>
#include <eflib/include/boostext.h>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/ref.hpp>
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

struct buildin_type;
struct constant_expression;
struct declaration;
struct declaration_statement;
struct expression;
struct function_type;
struct initializer;
struct node;
struct program;
struct type_specifier;
struct variable_declaration;

class dexpr_combinator;
class dvar_combinator;
class dtype_combinator;

#define SASL_TYPED_NODE_ACCESSORS_DECL( node_type )					\
	boost::shared_ptr< node_type > typed_node();	\
	template <typename T>	\
	boost::shared_ptr< node_type > typed_node( boost::shared_ptr< T > typed_ptr )  \
	{	\
		if ( typed_ptr ){ \
			cur_node = boost::shared_polymorphic_cast< node >( typed_ptr ); \
		} else { \
			cur_node.reset();\
		} \
		return typed_node(); \
	}	\
	template<typename T> boost::shared_ptr<T> typed_node2(){ \
		if ( cur_node )	return boost::shared_polymorphic_cast< T >( cur_node ); \
		return boost::shared_ptr<T>(); \
	}

#define SASL_TYPED_NODE_ACCESSORS_IMPL( class_name, node_type ) \
	boost::shared_ptr< node_type > class_name##::typed_node() { \
		return typed_node2< node_type >(); \
	}

class tree_combinator
{
public:
	virtual tree_combinator& dvar( const std::string& /*var_name*/ ){ return default_proc(); }
	virtual tree_combinator& dstruct( const std::string& /*struct_name*/ ){ return default_proc(); }
	virtual tree_combinator& dfunction( const std::string& /*func_name*/ ){ return default_proc(); }
	virtual tree_combinator& dtypedef( const std::string& /*alias*/ ){ return default_proc(); }

	virtual tree_combinator& end(){
		if( parent ){
			parent->child_ended();
			return *parent;
		}
		return *this;
	}
	
	virtual tree_combinator& dtype(){ return default_proc(); }
	virtual tree_combinator& dinit(){ return default_proc(); }

	// types
	virtual tree_combinator& dbuildin( buildin_type_code /*btc*/ ){ return default_proc(); }
	virtual tree_combinator& dvec( buildin_type_code /*comp_btc*/, size_t /*size*/ ){ return default_proc(); }
	virtual tree_combinator& dmat( buildin_type_code /*comp_btc*/, size_t /*s0*/, size_t /*s1*/ ){ return default_proc(); }
	virtual tree_combinator& dalias( const std::string& /*alias*/ ){ return default_proc(); }
	virtual tree_combinator& darray(){ return default_proc(); }
	virtual tree_combinator& dtypequal( type_qualifiers /*qual*/ ){ return default_proc(); }

	template <typename T>
	tree_combinator& end( boost::shared_ptr<T>& result )
	{
		result = boost::shared_polymorphic_cast<T>( cur_node );
		return end();
	}

	SASL_TYPED_NODE_ACCESSORS_DECL( node );
protected:
	tree_combinator( tree_combinator* parent ): parent( parent ){}
	tree_combinator& default_proc(){ syntax_error(); return *this; }

	virtual void child_ended(){}
	virtual ~tree_combinator(){}
private:
	tree_combinator( const tree_combinator& );
	tree_combinator& operator = ( const tree_combinator& );

	void syntax_error(){
		assert( !"Fuck!" );
	}

protected:
	boost::shared_ptr<node> cur_node;
	tree_combinator* parent;
};

class dprog_combinator: public tree_combinator{
public:
	dprog_combinator( const std::string& prog_name );

	virtual tree_combinator& dvar( const std::string& var_name );
	//virtual tree_combinator& dstruct( const std::string& struct_name );
	//virtual tree_combinator& dfunction( const std::string& func_name );
	//virtual tree_combinator& dtypedef( const std::string& alias );

	SASL_TYPED_NODE_ACCESSORS_DECL( program );

private:
	boost::shared_ptr<program> prog_node;
	boost::shared_ptr<dvar_combinator> var_comb;
};

class dvar_combinator: public tree_combinator{
public:
	explicit dvar_combinator( tree_combinator* parent );
	virtual tree_combinator& dname(const std::string& );
	virtual tree_combinator& dtype();
	//virtual tree_combinator& dinit();

	virtual void child_ended();
	SASL_TYPED_NODE_ACCESSORS_DECL( variable_declaration );
protected:
	enum child_state_t{
		e_none,
		e_type,
		e_init
	};
	dvar_combinator( const dvar_combinator& );
	dvar_combinator& operator = ( const dvar_combinator& );
private:
	child_state_t e_state;
	boost::shared_ptr<dtype_combinator> type_comb;
	// boost::shared_ptr<dinit_combinator> init_comb;
};

class dtype_combinator : public tree_combinator
{
public:

	dtype_combinator( tree_combinator* parent );
	~dtype_combinator(){}
	virtual tree_combinator& dbuildin( buildin_type_code btc );
	virtual tree_combinator& dvec( buildin_type_code comp_btc, size_t size );
	virtual tree_combinator& dmat( buildin_type_code comp_btc, size_t s0, size_t s1 );
	virtual tree_combinator& dalias( const std::string& alias );
	virtual tree_combinator& darray();
	virtual tree_combinator& dtypequal( type_qualifiers qual );

	SASL_TYPED_NODE_ACCESSORS_DECL( type_specifier );
protected:
	dtype_combinator( const dtype_combinator& rhs);
	dtype_combinator& operator = ( const dtype_combinator& rhs );

	virtual void child_ended();
private:
	boost::shared_ptr<dexpr_combinator> expr_comb;
	enum state_t{
		e_none,
		e_array
	} e_state;
};

class dexpr_combinator: public tree_combinator
{
public:
	dexpr_combinator( tree_combinator* parent );

	SASL_TYPED_NODE_ACCESSORS_DECL( expression );
protected:
	dexpr_combinator( const dexpr_combinator& rhs);
	dexpr_combinator& operator = ( const dexpr_combinator& rhs );
private:
};

END_NS_SASL_SYNTAX_TREE()
#endif