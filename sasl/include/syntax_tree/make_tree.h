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

#define SASL_TYPED_NODE_ACCESSORS( type )					\
	boost::shared_ptr< type > typed_node() { return boost::shared_polymorphic_cast< type >( cur_node ); }	\
	void typed_node( boost::shared_ptr< type > typed_ptr ) \
	{ cur_node = boost::shared_polymorphic_cast< node >( typed_ptr ); } \
	template<typename T> boost::shared_ptr<T> typed_node2{ return boost::shared_polymorphic_cast< T >( cur_node ); }

class tree_combinator
{
	virtual tree_combinator& dvar( const std::string& var_name ){ default_proc(); }
	virtual tree_combinator& dstruct( const std::string& struct_name ){ default_proc(); }
	virtual tree_combinator& dfunction( const std::string& func_name ){ default_proc(); }
	virtual tree_combinator& dtypedef( const std::string& alias ){ default_proc(); }

	virtual tree_combinator& end(){ parent.child_ended(); return parent; }
	
	virtual tree_combinator& dtype(){ default_proc(); }
	virtual tree_combinator& dinit(){ default_proc(); }

	// types
	virtual tree_combinator& dbuildin( buildin_type_code btc ){ default_proc(); }
	virtual tree_combinator& dvec( buildin_type_code comp_btc, size_t size ){ default_proc(); }
	virtual tree_combinator& dmat( buildin_type_code comp_btc, size_t s0, size_t s1 ){ default_proc(); }
	virtual tree_combinator& dalias( const std::string& alias ){ default_proc(); }
	virtual tree_combinator& darray(){ default_proc(); }
	virtual tree_combinator& dtypequal( type_qualifiers qual ){ default_proc(); }

	template <typename T>
	tree_combinator& end( boost::shared_ptr<T>& result )
	{
		result = boost::shared_polymorphic_cast<T>( cur_node );
		return end();
	}
protected:
	tree_combinator( tree_combinator& parent ): parent( parent ){}
	tree_combinator& default_proc(){ syntax_error(); return *this; }

	virtual void child_ended(){}
private:
	tree_combinator( const tree_combinator& );
	tree_combinator& operator = ( const tree_combinator& );

	void syntax_error(){
		assert( !"Fuck!" );
	}

protected:
	boost::shared_ptr<node> cur_node;
	tree_combinator& parent;
};

class dprog_combinator: public tree_combinator{
public:
	dprog_combinator();

	virtual tree_combinator& dvar( const std::string& var_name );
	//virtual tree_combinator& dstruct( const std::string& struct_name );
	//virtual tree_combinator& dfunction( const std::string& func_name );
	//virtual tree_combinator& dtypedef( const std::string& alias );

	virtual tree_combinator& child_ended();

	SASL_TYPED_NODE_ACCESSORS( program );

private:
	boost::shared_ptr<program> prog_node;
	boost::shared_ptr<dvar_combinator> var_comb;
};

class dvar_combinator: public tree_combinator{
public:
	explicit dvar_combinator( const std::string& var_name );
	virtual tree_combinator& dtype();
	//virtual tree_combinator& dinit();

	virtual tree_combinator& end();

	SASL_TYPED_NODE_ACCESSORS( variable_declaration );
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
	dtype_combinator();
	~dtype_combinator();
	virtual tree_combinator& dbuildin( buildin_type_code btc );
	/*virtual tree_combinator& dvec( buildin_type_code comp_btc, size_t size );
	virtual tree_combinator& dmat( buildin_type_code comp_btc, size_t s0, size_t s1 );
	virtual tree_combinator& dalias( const std::string& alias );
	virtual tree_combinator& darray();
	virtual tree_combinator& dtypequal( type_qualifiers qual );*/

	SASL_TYPED_NODE_ACCESSORS( type_specifier );
protected:
	dtype_combinator( const dtype_combinator& rhs);
	dtype_combinator& operator = ( const dtype_combinator& rhs );
private:
};
END_NS_SASL_SYNTAX_TREE()
#endif