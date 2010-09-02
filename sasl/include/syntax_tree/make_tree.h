#ifndef SASL_SYNTAX_TREE_MAKE_TREE_H
#define SASL_SYNTAX_TREE_MAKE_TREE_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>

#include <sasl/enums/buildin_type_code.h>
#include <sasl/enums/literal_constant_types.h>
#include <sasl/enums/operators.h>
#include <sasl/enums/type_qualifiers.h>
#include <sasl/include/syntax_tree/node_creation.h>
#include <eflib/include/boostext.h>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/set.hpp>
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

struct typecode_map
{
	typedef boost::mpl::vector<
		bool, 
		int8_t, uint8_t, 
		int16_t, uint16_t,
		int32_t, uint32_t,
		int64_t, uint64_t,
		float, double
	> cpptypes;

	static literal_constant_types type_codes[];

	template<typename T>
	struct is_sasl_buildin_type: public boost::mpl::not_< 
		boost::is_same<
		typename boost::mpl::find<cpptypes, T>::type,
		typename boost::mpl::end<cpptypes>::type 
		>
	>::type{};

	template <typename T>
	static literal_constant_types lookup( EFLIB_ENABLE_IF_COND( is_sasl_buildin_type<T>, 0 ) )
	{
		return type_codes[boost::mpl::find<cpptypes, T>::type::pos::value];
	}
};

struct binary_expression;
struct buildin_type;
struct call_expression;
struct cast_expression;
struct compound_statement;
struct cond_expression;
struct constant_expression;
struct declaration;
struct declaration_statement;
struct expression;
struct function_type;
struct initializer;
struct node;
struct program;
struct struct_type;
struct type_specifier;
struct variable_declaration;

class dbinexpr_combinator;
class dbranchexpr_combinator;
class dcast_combinator;
class dcallexpr_combinator;
class dexpr_combinator;
class dstruct_combinator;
class dtype_combinator;
class dvar_combinator;

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

	// expressions
	virtual tree_combinator& dexpr(){ return default_proc(); }
	virtual tree_combinator& dconstant( literal_constant_types /*lct*/, const std::string& /*v*/ ){
		return default_proc();
	}
	// impl by expression
	template <typename T> tree_combinator& dconstant2(
		const T& v,
		EFLIB_ENABLE_IF_COND( typecode_map::is_sasl_buildin_type<T>, 0 ) 
		)
	{
		return dconstant( typecode_map::lookup<T>(), boost::lexical_cast<std::string>(v) );
	}
	virtual tree_combinator& dvarexpr( const std::string& /*v*/){ return default_proc(); }
	virtual tree_combinator& dunary( operators /*op*/ ){ return default_proc(); };
	virtual tree_combinator& dlexpr(){ return default_proc(); }
	virtual tree_combinator& dop( operators /*op*/){ return default_proc(); }
	virtual tree_combinator& drexpr(){ return default_proc(); }
	virtual tree_combinator& dbranchexpr(){ return default_proc(); }
	virtual tree_combinator& dcond(){ return default_proc(); }
	virtual tree_combinator& dyes(){ return default_proc(); }
	virtual tree_combinator& dno(){ return default_proc(); }
	virtual tree_combinator& dmember( const std::string& /*m*/){ return default_proc(); }
	virtual tree_combinator& dcall(){ return default_proc(); }
	virtual tree_combinator& dargument(){ return default_proc(); }
	virtual tree_combinator& dindex(){ return default_proc(); }

	template <typename T>
	tree_combinator& end( boost::shared_ptr<T>& result )
	{
		get_node<T>(result);
		return end();
	}

	template <typename T>
	tree_combinator& dnode( boost::shared_ptr<T> node )
	{
		enter( e_other );
		typed_node( node );
		leave();
		return *this;
	}
	template<typename T>
	tree_combinator& get_node( boost::shared_ptr<T>& result ){
		result = typed_node2<T>();
		return *this;
	}

	SASL_TYPED_NODE_ACCESSORS_DECL( node );
protected:
	enum state_t{
		e_none,

		e_vardecl,
		e_struct,

		e_type,
		e_init,

		e_array,

		e_expr,
		e_unary,
		e_cast,
		e_binexpr,
		e_lexpr,
		e_binop,
		e_rexpr,
		e_branchexpr,
		e_cond,
		e_yes,
		e_no,
		e_callexpr,
		e_argument,
		e_indexexpr,

		e_other = UINT_MAX
	};

	void enter( state_t s ){
		assert( s != e_none );
		assert( e_state == e_none );
		e_state = s;
	}
	state_t leave(){
		assert( e_state != e_none );
		state_t ret = e_state;
		e_state = e_none;
		return ret;
	}
	bool is_state( state_t s){
		return e_state == s;
	}

	class state_scope
	{
	public:
		state_scope( tree_combinator* owner, state_t s ):owner(owner){
			owner->enter( s );
		}
		~state_scope(){
			owner->leave();
		}
	private:
		tree_combinator* owner;
		state_scope( const state_scope& rhs);
		state_scope& operator = ( const state_scope& rhs );
	};

	tree_combinator( tree_combinator* parent ): parent( parent ), e_state( e_none ){}
	tree_combinator& default_proc(){ syntax_error(); return *this; }
	
	template< typename T >
	tree_combinator& enter_child( state_t s, boost::shared_ptr<T>& child_comb, bool comb_reusable = false ){
		assert( comb_reusable || !child_comb );
		enter(s);
		child_comb = boost::make_shared<T>(this);
		return *child_comb;
	}

	virtual void child_ended(){}
	virtual ~tree_combinator(){ assert( is_state(e_none) );}
private:
	state_t e_state;

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

	virtual tree_combinator& dvar( const std::string& /*var_name*/ );
	virtual tree_combinator& dstruct( const std::string& /*struct_name*/ );
	//virtual tree_combinator& dfunction( const std::string& func_name );
	//virtual tree_combinator& dtypedef( const std::string& alias );

	virtual void child_ended();

	SASL_TYPED_NODE_ACCESSORS_DECL( program );

private:
	boost::shared_ptr<program> prog_node;
	boost::shared_ptr<dvar_combinator> var_comb;
	boost::shared_ptr<dstruct_combinator> struct_comb;
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
	dvar_combinator( const dvar_combinator& );
	dvar_combinator& operator = ( const dvar_combinator& );
private:
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
};

class dexpr_combinator: public tree_combinator
{
public:
	dexpr_combinator( tree_combinator* parent );

	virtual tree_combinator& dconstant( literal_constant_types /*lct*/, const std::string& /*v*/ );
	virtual tree_combinator& dvarexpr( const std::string& /*v*/);
	virtual tree_combinator& dunary( operators op );
	virtual tree_combinator& dcast();
	virtual tree_combinator& dbinary();
	virtual tree_combinator& dbranchexpr();
	virtual tree_combinator& dmember( const std::string& /*m*/);
	virtual tree_combinator& dcall();
	virtual tree_combinator& dindex();

	SASL_TYPED_NODE_ACCESSORS_DECL( expression );
protected:
	dexpr_combinator( const dexpr_combinator& rhs);
	dexpr_combinator& operator = ( const dexpr_combinator& rhs );
	
	virtual void child_ended();
private:
	boost::shared_ptr<dcast_combinator>			cast_comb;
	boost::shared_ptr<dexpr_combinator>			expr_comb;
	boost::shared_ptr<dbinexpr_combinator>		binexpr_comb;
	boost::shared_ptr<dbranchexpr_combinator>	branch_comb;
	boost::shared_ptr<dcallexpr_combinator>		call_comb;
};

class dcast_combinator: public tree_combinator{
public:
	dcast_combinator( tree_combinator* parent );

	virtual tree_combinator& dtype();
	virtual tree_combinator& dexpr();

	SASL_TYPED_NODE_ACCESSORS_DECL( cast_expression );
protected:
	dcast_combinator( const dcast_combinator& rhs);
	dcast_combinator& operator = ( const dcast_combinator& rhs );

	virtual void child_ended();
private:
	boost::shared_ptr<dexpr_combinator>	expr_comb;
	boost::shared_ptr<dtype_combinator>	type_comb;
};

class dbinexpr_combinator: public tree_combinator
{
public:
	dbinexpr_combinator( tree_combinator* parent);

	virtual tree_combinator& dlexpr();
	virtual tree_combinator& dop( operators /*op*/);
	virtual tree_combinator& drexpr();

	SASL_TYPED_NODE_ACCESSORS_DECL( binary_expression );
protected:
	dbinexpr_combinator( const dbinexpr_combinator& rhs);
	dbinexpr_combinator& operator = ( const dbinexpr_combinator& rhs );

	virtual void child_ended();
private:
	boost::shared_ptr<dexpr_combinator> lexpr_comb;
	boost::shared_ptr<dexpr_combinator> rexpr_comb;	
};

class dbranchexpr_combinator: public tree_combinator
{
public:
	dbranchexpr_combinator( tree_combinator* parent );
	
	virtual tree_combinator& dcond();
	virtual tree_combinator& dyes();
	virtual tree_combinator& dno();

	SASL_TYPED_NODE_ACCESSORS_DECL( cond_expression );
protected:
	dbranchexpr_combinator( const dbranchexpr_combinator& rhs);
	dbranchexpr_combinator& operator = ( const dbranchexpr_combinator& rhs );

	virtual void child_ended();
private:
	boost::shared_ptr<dexpr_combinator>
		cond_comb, yes_comb, no_comb;
};

class dcallexpr_combinator : public tree_combinator
{
public:
	dcallexpr_combinator( tree_combinator* parent );

	virtual tree_combinator& dargument();

	SASL_TYPED_NODE_ACCESSORS_DECL( call_expression );
protected:
	dcallexpr_combinator( const dcallexpr_combinator& rhs);
	dcallexpr_combinator& operator = ( const dcallexpr_combinator& rhs );

	virtual void child_ended();
private:
	boost::shared_ptr<dexpr_combinator> argexpr;
};

// struct combinator
class dstruct_combinator: public tree_combinator
{
public:
	dstruct_combinator( tree_combinator* parent );

	virtual tree_combinator& dname( const std::string& /*struct name*/);
	virtual tree_combinator& dmember( const std::string& /*var name*/);

	virtual void child_ended();

	SASL_TYPED_NODE_ACCESSORS_DECL( struct_type );
protected:
	dstruct_combinator( const dstruct_combinator& rhs);
	dstruct_combinator& operator = ( const dstruct_combinator& rhs );
private:
	boost::shared_ptr<dvar_combinator> var_comb;
};

// statement combinators
//class dstatements_combinator: public tree_combinator
//{
//public:
//	dstatements_combinator( tree_combinator* parent );
//
//	virtual tree_combinator& dvarstmt();
//	virtual tree_combinator& dexpr();
//
//	virtual tree_combinator& dif();
//
//	virtual tree_combinator& ddo();
//	virtual tree_combinator& dwhile();
//	virtual tree_combinator& dswitch();
//	virtual tree_combinator& dfor();
//
//	virtual tree_combinator& dbreak();
//	virtual tree_combinator& dcontinue();
//	virtual tree_combinator& dreturn();
//
//	SASL_TYPED_NODE_ACCESSORS_DECL( compound_statement );
//
//protected:
//	dstatements_combinator( const dstatements_combinator& rhs);
//	dstatements_combinator& operator = ( const dstatements_combinator& rhs );
//private:
//};

END_NS_SASL_SYNTAX_TREE()
#endif