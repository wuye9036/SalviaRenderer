#ifndef SASL_SYNTAX_TREE_MAKE_TREE_H
#define SASL_SYNTAX_TREE_MAKE_TREE_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/literal_classifications.h>
#include <sasl/enums/operators.h>
#include <sasl/enums/type_qualifiers.h>
#include <sasl/include/syntax_tree/node_creation.h>
#include <eflib/include/utility/enable_if.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/ref.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <string>
#include <vector>

namespace sasl{
	namespace common{
		struct token_t;
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

	static const literal_classifications* const type_codes();

	template<typename T>
	struct is_sasl_builtin_type: public boost::mpl::not_<
		boost::is_same<
		typename boost::mpl::find<cpptypes, T>::type,
		typename boost::mpl::end<cpptypes>::type
		>
	>::type{};

	template <typename T>
	static literal_classifications lookup( EFLIB_ENABLE_IF_COND( is_sasl_builtin_type<T> ) )
	{
		return type_codes()[boost::mpl::find<cpptypes, T>::type::pos::value];
	}
};

struct binary_expression;
struct builtin_type;
struct call_expression;
struct case_label;
struct cast_expression;
struct compound_statement;
struct cond_expression;
struct constant_expression;
struct declaration;
struct declaration_statement;
struct declarator;
struct dowhile_statement;
struct expression;
struct function_type;
struct for_statement;
struct if_statement;
struct ident_label;
struct initializer;
struct jump_statement;
struct label;
struct member_initializer;
struct node;
struct program;
struct struct_type;
struct switch_statement;
struct type_definition;
struct tynode;
struct variable_declaration;
struct while_statement;

class dbinexpr_combinator;
class dbranchexpr_combinator;
class dcase_combinator;
class dcast_combinator;
class dcallexpr_combinator;
class ddeclarator_combinator;
class ddowhile_combinator;
class dexpr_combinator;
class dexprstmt_combinator;
class dfor_combinator;
class dfunction_combinator;
class dif_combinator;
class dinitexpr_combinator;
class dinitlist_combinator;
class dparameter_combinator;
class dreturn_combinator;
class dstruct_combinator;
class dswitch_combinator;
class dswitchbody_combinator;
class dtype_combinator;
class dtypedef_combinator;
class dvar_combinator;
class dvarstmt_combinator;
class dwhiledo_combinator;

#define SASL_TYPED_NODE_ACCESSORS_DECL( node_type )					\
	typedef node_type node_t;	\
	boost::shared_ptr< node_type > typed_node();	\
	template <typename T>	\
	void typed_node( boost::shared_ptr< T > typed_ptr )  \
	{	\
		if ( typed_ptr ){ \
			cur_node = boost::shared_polymorphic_cast< node >( typed_ptr ); \
		} else { \
			cur_node.reset();\
		} \
	}	\
	template<typename T> boost::shared_ptr<T> typed_node2(){ \
		if ( cur_node )	return boost::shared_polymorphic_cast< T >( cur_node ); \
		return boost::shared_ptr<T>(); \
	}

#define SASL_TYPED_NODE_ACCESSORS_IMPL( class_name, node_type ) \
	boost::shared_ptr< node_type > class_name::typed_node() { \
		return typed_node2< node_type >(); \
	}

/////////////////////////////////////////////////////////////////////////////////////
// Tree combinator is a helper class for syntax tree generation.
// 1. Motion
//   Combinator is a good method for creating a compisite tree.
//   In our design, each combinator will generate a type of syntax tree node.
//   These combinators inherit from a root class, tree_combinator, which supports
//   the interface of all combinators.
//   Others, the concreted combinators implement specified nodes creation.

//   We use function prefix with "d" character for changing the type of combinator and
//   do the code generation. And the returned value is a reference of tree_combinator referred
//   to this or any combinator instance. So the combinator can be used as sequence.
class tree_combinator
{
public:
	virtual tree_combinator& dname( const std::string& /*name*/){ return default_proc(); }

	virtual tree_combinator& dvar(){ return default_proc(); }
	virtual tree_combinator& dstruct( const std::string& /*struct_name*/ ){ return default_proc(); }
	virtual tree_combinator& dfunction( const std::string& /*func_name*/ ){ return default_proc(); }
	virtual tree_combinator& dreturntype(){ return default_proc(); }
	virtual tree_combinator& dparam(){ return default_proc(); }

	virtual tree_combinator& dtypedef(){ return default_proc(); }


	virtual tree_combinator& dtype(){ return default_proc(); }
	virtual tree_combinator& dinit_expr(){ return default_proc(); }
	virtual tree_combinator& dinit_list(){ return default_proc(); }

	// types
	virtual tree_combinator& dbuiltin( builtin_types /*btc*/ ){ return default_proc(); }
	virtual tree_combinator& dvec( builtin_types /*comp_btc*/, size_t /*size*/ ){ return default_proc(); }
	virtual tree_combinator& dmat( builtin_types /*comp_btc*/, size_t /*s0*/, size_t /*s1*/ ){ return default_proc(); }
	virtual tree_combinator& dalias( const std::string& /*alias*/ ){ return default_proc(); }
	virtual tree_combinator& darray(){ return default_proc(); }
	virtual tree_combinator& dtypequal( type_qualifiers /*qual*/ ){ return default_proc(); }

	// expressions
	virtual tree_combinator& dexpr(){ return default_proc(); }
	virtual tree_combinator& dconstant( literal_classifications /*lct*/, const std::string& /*v*/ ){
		return default_proc();
	}
	// impl by expression
	template <typename T> tree_combinator& dconstant2(
		const T& v,
		EFLIB_ENABLE_IF_COND( typecode_map::is_sasl_builtin_type<T> )
		)
	{
		std::string suffix;
		append_suffix<T>(suffix);

		literal_classifications lct = typecode_map::lookup<T>();
		return dconstant( lct, boost::lexical_cast<std::string>(v) + suffix );
	}
	virtual tree_combinator& dvarexpr( const std::string& /*v*/){ return default_proc(); }
	virtual tree_combinator& dcast(){ return default_proc(); }
	virtual tree_combinator& dunary( operators /*op*/ ){ return default_proc(); }
	virtual tree_combinator& dbinary(){ return default_proc(); }
	virtual tree_combinator& dlexpr(){ return default_proc(); }
	virtual tree_combinator& dop( operators /*op*/){ return default_proc(); }
	virtual tree_combinator& drexpr(){ return default_proc(); }
	virtual tree_combinator& dbranchexpr(){ return default_proc(); }
	virtual tree_combinator& dcond(){ return default_proc(); }
	virtual tree_combinator& dyes(){ return default_proc(); }
	virtual tree_combinator& dno(){ return default_proc(); }
	virtual tree_combinator& dmember(){ return default_proc(); } // For struct definition only.
	virtual tree_combinator& dmember( std::string const & /*m*/ ){ return default_proc(); } // For member-expr only.
	virtual tree_combinator& dcall(){ return default_proc(); }
	virtual tree_combinator& dargument(){ return default_proc(); }
	virtual tree_combinator& dindex(){ return default_proc(); }
	

	virtual tree_combinator& dlabel( const std::string& /*v*/ ){ return default_proc(); }
	virtual tree_combinator& dvarstmt(){ return default_proc(); }
	virtual tree_combinator& dexprstmt(){ return default_proc(); }
	virtual tree_combinator& dif(){return default_proc();}
	virtual tree_combinator& dthen(){return default_proc();}
	virtual tree_combinator& delse(){return default_proc();}
	virtual tree_combinator& ddowhile(){ return default_proc(); }
	virtual tree_combinator& dwhiledo(){ return default_proc(); }
	virtual tree_combinator& ddo(){ return default_proc(); }
	virtual tree_combinator& dwhile(){ return default_proc(); }
	virtual tree_combinator& dswitch(){ return default_proc(); }
	virtual tree_combinator& dbody(){ return default_proc(); }
	virtual tree_combinator& dcase(){ return default_proc(); }
	virtual tree_combinator& ddefault(){ return default_proc(); }
	virtual tree_combinator& dstmts(){ return default_proc(); }

	virtual tree_combinator& dfor(){ return default_proc(); }
	virtual tree_combinator& dinit_var(){ return default_proc(); }
	virtual tree_combinator& diter(){ return default_proc(); }

	virtual tree_combinator& dbreak(){ return default_proc(); }
	virtual tree_combinator& dcontinue(){ return default_proc(); }
	virtual tree_combinator& dreturn_expr(){ return default_proc(); }
	virtual tree_combinator& dreturn_void(){ return default_proc(); }

	//////////////////////////////////////////////////////////////////////////
	// end of node
	virtual tree_combinator& end(){	before_end(); return do_end(); }
	template <typename T>
	tree_combinator& end( boost::shared_ptr<T>& result )
	{
		// If assertion at here
		// please confirm that the type of "result" is as same as type of node.
		before_end();
		get_node<T>(result);
		return do_end();
	}

	//////////////////////////////////////////////////////////////////////////
	// accessors
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
		e_declarator,
		e_struct,
		e_function,
		e_param,
		e_typedef,

		e_type,
		e_init,
		e_initexpr,
		e_initlist,

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

		e_compound,
		e_varstmt,
		e_exprstmt,
		e_if,
		e_then,
		e_else,
		e_switch,
		e_switchbody,
		e_case,
		e_default,
		e_for,
		e_iter,
		e_body,
		e_do,
		e_while,
		e_whiledo,
		e_dowhile,
		e_return,

		e_other = UINT_MAX
	};

	void enter( state_t s );
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
	tree_combinator& enter_child( state_t s, boost::shared_ptr<T>& child_comb ){
		assert( !child_comb );
		enter(s);
		child_comb = boost::make_shared<T>(this);
		return *child_comb;
	}

	template<typename T, typename U>
	static boost::shared_ptr<T> move_node2( boost::shared_ptr<U>& comb, bool enable_null = true ){
		assert( comb );
		boost::shared_ptr<T> ret = comb->typed_node2<T>();
		assert( ret || enable_null );
		comb.reset();
		return ret;
	}

	template<typename T>
	static boost::shared_ptr< typename T::node_t > move_node( boost::shared_ptr<T>& comb, bool enable_null = true ){
		assert( comb );
		boost::shared_ptr<typename T::node_t> ret = comb->typed_node2<typename T::node_t>();
		assert( ret || enable_null );
		comb.reset();
		return ret;
	}

	virtual void child_ended(){}
	virtual void before_end(){}
	virtual tree_combinator& do_end();
	virtual ~tree_combinator();
private:
	state_t e_state;

	tree_combinator( const tree_combinator& );
	tree_combinator& operator = ( const tree_combinator& );

	void syntax_error();

	template <typename T>
	class suffix_traits{
		typedef boost::mpl::and_< boost::is_integral<T>, boost::mpl::not_< boost::is_same<T, bool> > > is_int;
		typedef boost::mpl::and_<
			is_int,
			boost::is_unsigned<T>
		> u;
		typedef boost::mpl::and_<
			is_int,
			boost::mpl::bool_<sizeof(T) == sizeof(int64_t)>
		> l;
		typedef	boost::is_same< T, float > f;
	};

	template <typename T>
	void append_suffix(
		std::string& out,
		EFLIB_ENABLE_IF_COND(typename suffix_traits<T>::u),
		EFLIB_DISABLE_IF_COND(typename suffix_traits<T>::l)
		)
	{
		out += "u";
	}
	
	template <typename T>
	void append_suffix(
		std::string& out,
		EFLIB_DISABLE_IF_COND(typename suffix_traits<T>::u),
		EFLIB_ENABLE_IF_COND(typename suffix_traits<T>::l)
		)
	{
		out += "l";
	}

	template <typename T>
	void append_suffix(
		std::string& out,
		EFLIB_ENABLE_IF_COND(typename suffix_traits<T>::u),
		EFLIB_ENABLE_IF_COND(typename suffix_traits<T>::l)
		)
	{
		out += "ul";
	}

	template <typename T>
	void append_suffix(
		std::string& out,
		EFLIB_ENABLE_IF_COND(typename suffix_traits<T>::f)
		)
	{
		out += "f";
	}

	template <typename T>
	void append_suffix( std::string& /*out*/,
		EFLIB_DISABLE_IF_COND(typename suffix_traits<T>::u),
		EFLIB_DISABLE_IF_COND(typename suffix_traits<T>::l),
		EFLIB_DISABLE_IF_COND(typename suffix_traits<T>::f)
		)
	{
		// do nothing.
	}
protected:
	boost::shared_ptr<node> cur_node;
	tree_combinator* parent;
};

class dprog_combinator: public tree_combinator{
public:
	dprog_combinator( const std::string& prog_name );

	virtual tree_combinator& dvar();
	virtual tree_combinator& dstruct( const std::string& /*struct_name*/ );
	virtual tree_combinator& dfunction( const std::string& func_name );
	virtual tree_combinator& dtypedef( );

	virtual void child_ended();

	SASL_TYPED_NODE_ACCESSORS_DECL( program );

private:
	boost::shared_ptr<program> prog_node;
	boost::shared_ptr<dvar_combinator> var_comb;
	boost::shared_ptr<dstruct_combinator> struct_comb;
	boost::shared_ptr<dfunction_combinator> func_comb;
	boost::shared_ptr<dtypedef_combinator> typedef_comb;
};

class dvar_combinator: public tree_combinator{
public:
	explicit dvar_combinator( tree_combinator* parent );
	
	virtual tree_combinator& dtype();
	virtual tree_combinator& dname( const std::string& );
	virtual void child_ended();

	SASL_TYPED_NODE_ACCESSORS_DECL( variable_declaration );
protected:
	dvar_combinator( const dvar_combinator& );
	dvar_combinator& operator = ( const dvar_combinator& );
private:
	boost::shared_ptr<dtype_combinator> type_comb;
	boost::shared_ptr<ddeclarator_combinator> declarator_comb;
};

class ddeclarator_combinator: public tree_combinator{
public:
	explicit ddeclarator_combinator( tree_combinator* parent );

	virtual tree_combinator& dname(const std::string& );
	virtual tree_combinator& dinit_expr();
	virtual tree_combinator& dinit_list();
	virtual void child_ended();

	SASL_TYPED_NODE_ACCESSORS_DECL( declarator );
protected:
	ddeclarator_combinator( const dvar_combinator& );
	ddeclarator_combinator& operator = ( const dvar_combinator& );
private:
	boost::shared_ptr<dinitexpr_combinator> exprinit_comb;
	boost::shared_ptr<dinitlist_combinator> listinit_comb;
};

class dtype_combinator : public tree_combinator
{
public:

	dtype_combinator( tree_combinator* parent );
	~dtype_combinator(){}
	virtual tree_combinator& dbuiltin( builtin_types btc );
	virtual tree_combinator& dvec( builtin_types comp_btc, size_t size );
	virtual tree_combinator& dmat( builtin_types comp_btc, size_t s0, size_t s1 );
	virtual tree_combinator& dalias( const std::string& alias );
	virtual tree_combinator& darray();
	virtual tree_combinator& dtypequal( type_qualifiers qual );

	SASL_TYPED_NODE_ACCESSORS_DECL( tynode );
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

	virtual tree_combinator& dconstant( literal_classifications /*lct*/, const std::string& /*v*/ );
	virtual tree_combinator& dvarexpr( const std::string& /*v*/);
	virtual tree_combinator& dunary( operators op );
	virtual tree_combinator& dcast();
	virtual tree_combinator& dbinary();
	virtual tree_combinator& dbranchexpr();
	virtual tree_combinator& dmember( std::string const & m );
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
	virtual tree_combinator& dmember();

	virtual void child_ended();

	SASL_TYPED_NODE_ACCESSORS_DECL( struct_type );
protected:
	dstruct_combinator( const dstruct_combinator& rhs);
	dstruct_combinator& operator = ( const dstruct_combinator& rhs );
private:
	boost::shared_ptr<dvar_combinator> var_comb;
};

// statement combinators
class dstatements_combinator: public tree_combinator
{
public:
	dstatements_combinator( tree_combinator* parent );

	virtual tree_combinator& dlabel( const std::string& /*lbl*/ );

	virtual tree_combinator& dvarstmt();
	virtual tree_combinator& dexprstmt();
	virtual tree_combinator& dif();

	virtual tree_combinator& ddowhile();
	virtual tree_combinator& dwhiledo();

	virtual tree_combinator& dswitch();
	virtual tree_combinator& dfor();
	virtual tree_combinator& dbreak();
	virtual tree_combinator& dcontinue();
	virtual tree_combinator& dreturn_expr();
	virtual tree_combinator& dreturn_void();

	virtual tree_combinator& dstmts();

	virtual void child_ended();

	SASL_TYPED_NODE_ACCESSORS_DECL( compound_statement );

protected:
	dstatements_combinator( const dstatements_combinator& rhs);
	dstatements_combinator& operator = ( const dstatements_combinator& rhs );

	template <typename T> void push_label( boost::shared_ptr<T> lbl ){
		lbls.push_back( boost::shared_polymorphic_cast<label>(lbl) );
	}

private:
	boost::shared_ptr<dvarstmt_combinator> var_comb;
	boost::shared_ptr<dexprstmt_combinator> expr_comb;
	boost::shared_ptr<dif_combinator> if_comb;
	boost::shared_ptr<ddowhile_combinator> dowhile_comb;
	boost::shared_ptr<dwhiledo_combinator> whiledo_comb;
	boost::shared_ptr<dswitch_combinator> switch_comb;
	boost::shared_ptr<dreturn_combinator> ret_comb;
	boost::shared_ptr<dfor_combinator> for_comb;
	boost::shared_ptr<dstatements_combinator> compound_comb;

	std::vector< boost::shared_ptr<label> > lbls;
};

class dvarstmt_combinator: public dvar_combinator{
public:
	dvarstmt_combinator( tree_combinator* parent );
protected:
	//////////////////////////////////////////////////////////////////////////
	// before_end is called before end() method invoked.
	// it wraps the variable_declaration node to variable statement node.
	// so typed_node() cannot be invoked here.
	// typed_node2() could be executed only for specifying node type explicit.
	virtual void before_end();
private:
};

class dexprstmt_combinator: public dexpr_combinator{
public:
	dexprstmt_combinator( tree_combinator* parent );
protected:
	virtual void before_end();
};

class dif_combinator: public tree_combinator
{
public:
	dif_combinator( tree_combinator* parent );

	virtual tree_combinator& dcond();
	virtual tree_combinator& dthen();
	virtual tree_combinator& delse();

	virtual void child_ended();

	SASL_TYPED_NODE_ACCESSORS_DECL( if_statement );
protected:
	dif_combinator( const dif_combinator& rhs);
	dif_combinator& operator = ( const dif_combinator& rhs );

private:
	boost::shared_ptr<dexpr_combinator> expr_comb;
	boost::shared_ptr<dstatements_combinator> then_stmt_comb;
	boost::shared_ptr<dstatements_combinator> else_stmt_comb;
};

//////////////////////////////////////////////////////////////////////////
// do while combinator.
class ddowhile_combinator: public tree_combinator
{
public:
	ddowhile_combinator( tree_combinator* parent );

	virtual tree_combinator& ddo();
	virtual tree_combinator& dwhile();

	virtual void child_ended();

	// supply type-specified node accessors.
	SASL_TYPED_NODE_ACCESSORS_DECL( dowhile_statement );
protected:
	ddowhile_combinator( const ddowhile_combinator& rhs);
	ddowhile_combinator& operator = ( const ddowhile_combinator& rhs );
private:
	boost::shared_ptr<dstatements_combinator> do_comb;
	boost::shared_ptr<dexpr_combinator> cond_comb;
};

class dwhiledo_combinator: public tree_combinator
{
public:
	dwhiledo_combinator( tree_combinator* parent );

	virtual tree_combinator& ddo();
	virtual tree_combinator& dwhile();

	virtual void child_ended();

	// supply type-specified node accessors.
	SASL_TYPED_NODE_ACCESSORS_DECL( while_statement );
protected:
	dwhiledo_combinator( const dwhiledo_combinator& rhs);
	dwhiledo_combinator& operator = ( const dwhiledo_combinator& rhs );
private:
	boost::shared_ptr<dstatements_combinator> do_comb;
	boost::shared_ptr<dexpr_combinator> cond_comb;
};


class dswitch_combinator : public tree_combinator
{
public:
	dswitch_combinator( tree_combinator* parent );

	virtual tree_combinator& dexpr();
	virtual tree_combinator& dbody();

	virtual void child_ended();

	SASL_TYPED_NODE_ACCESSORS_DECL( switch_statement );
protected:
	dswitch_combinator( const dswitch_combinator& rhs);
	dswitch_combinator& operator = ( const dswitch_combinator& rhs );
private:
	boost::shared_ptr<dexpr_combinator> expr_comb;
	boost::shared_ptr<dswitchbody_combinator> body_comb;
};

class dswitchbody_combinator: public dstatements_combinator
{
public:
	dswitchbody_combinator( tree_combinator* parent );

	virtual tree_combinator& dcase();
	virtual tree_combinator& ddefault();

	virtual void child_ended();

protected:
	dswitchbody_combinator( const dswitchbody_combinator& /*rhs*/ );
	dswitchbody_combinator& operator = ( const dswitchbody_combinator& /*rhs*/ );
private:
	boost::shared_ptr<dcase_combinator> case_comb;
};

class dcase_combinator: public dexpr_combinator
{
public:
	dcase_combinator( tree_combinator* parent );
protected:
	virtual void before_end();
private:
	dcase_combinator( const dcase_combinator& /*rhs*/ );
	dcase_combinator& operator = ( const dcase_combinator& /*rhs*/ );
};

class dreturn_combinator : public dexpr_combinator
{
public:
	dreturn_combinator( tree_combinator* parent );
protected:
	virtual void before_end();
private:
	dreturn_combinator( const dreturn_combinator& rhs);
	dreturn_combinator& operator = ( const dreturn_combinator& rhs );
};


class dfor_combinator : public tree_combinator
{
public:
	SASL_TYPED_NODE_ACCESSORS_DECL( for_statement );

	dfor_combinator( tree_combinator* parent );

	virtual tree_combinator& dinit_expr();
	virtual tree_combinator& dinit_var();
	virtual tree_combinator& dcond();
	virtual tree_combinator& diter();
	virtual tree_combinator& dbody();

	virtual void child_ended();
protected:
	dfor_combinator( const dfor_combinator& rhs);
	dfor_combinator& operator = ( const dfor_combinator& rhs );
private:
	boost::shared_ptr<dexprstmt_combinator> initexpr_comb;
	boost::shared_ptr<dvarstmt_combinator> initvar_comb;
	boost::shared_ptr<dexpr_combinator> cond_comb;
	boost::shared_ptr<dexpr_combinator> iter_comb;
	boost::shared_ptr<dstatements_combinator> body_comb;
};

//////////////////////////////////////////////////////////////////////////
// function combinator
class dfunction_combinator: public tree_combinator
{
public:
	SASL_TYPED_NODE_ACCESSORS_DECL( function_type );

	dfunction_combinator( tree_combinator* parent );

	virtual tree_combinator& dname( const std::string& str );
	virtual tree_combinator& dreturntype();
	virtual tree_combinator& dparam();
	virtual tree_combinator& dbody();

	virtual void child_ended();
protected:
	dfunction_combinator( const dfunction_combinator& rhs);
	dfunction_combinator& operator = ( const dfunction_combinator& rhs );
private:
	boost::shared_ptr<dtype_combinator> rettype_comb;
	boost::shared_ptr<dparameter_combinator> par_comb;
	boost::shared_ptr<dstatements_combinator> body_comb;
};

class dparameter_combinator: public dvar_combinator
{
public:
	dparameter_combinator( tree_combinator* parent );
protected:
	dparameter_combinator( const dparameter_combinator& rhs);
	dparameter_combinator& operator = ( const dparameter_combinator& rhs );

	virtual void before_end();
};

class dtypedef_combinator: public tree_combinator
{
public:
	dtypedef_combinator( tree_combinator* parent );

	virtual tree_combinator& dname( const std::string& /*name*/ );
	virtual tree_combinator& dtype();

	virtual void child_ended();

	SASL_TYPED_NODE_ACCESSORS_DECL( type_definition );
protected:
	dtypedef_combinator( const dtypedef_combinator& rhs);
	dtypedef_combinator& operator = ( const dtypedef_combinator& rhs );
private:
	boost::shared_ptr<dtype_combinator> type_comb;
};

class dinitexpr_combinator : public dexpr_combinator
{
public:
	dinitexpr_combinator( tree_combinator* parent );

	virtual void before_end();
protected:
	dinitexpr_combinator( const dinitexpr_combinator& rhs);
	dinitexpr_combinator& operator = ( const dinitexpr_combinator& rhs );
};

class dinitlist_combinator: public tree_combinator
{
public:
	dinitlist_combinator( tree_combinator* parent );

	virtual tree_combinator& dinit_expr();
	virtual tree_combinator& dinit_list();

	virtual void child_ended();

	SASL_TYPED_NODE_ACCESSORS_DECL( member_initializer );

protected:
	dinitlist_combinator( const dinitlist_combinator& rhs);
	dinitlist_combinator& operator = ( const dinitlist_combinator& rhs );
private:
	boost::shared_ptr<dinitlist_combinator> list_comb;
	boost::shared_ptr<dinitexpr_combinator> expr_comb;
};

END_NS_SASL_SYNTAX_TREE()
#endif
