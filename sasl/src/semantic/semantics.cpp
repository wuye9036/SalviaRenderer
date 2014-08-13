#include <sasl/include/semantic/semantics.h>

#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/pety.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/common/diag_chat.h>
#include <sasl/enums/builtin_types.h>
#include <salviar/include/shader.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/pool/pool.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

#include <map>

EFLIB_USING_SHARED_PTR(sasl::syntax_tree, program);
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, node);
EFLIB_USING_SHARED_PTR(sasl::common, diag_chat);

using boost::unordered_map;
using boost::unordered_set;
using boost::shared_ptr;
using boost::make_shared;
using boost::string_ref;

using std::vector;
using std::string;
using std::make_pair;
using std::map;

string_ref split_integer_literal_suffix(string_ref str, bool& is_unsigned, bool& is_long)
{
	is_unsigned = false;
	is_long = false;

	size_t tail_count = 0;

	for(auto ch_it = str.rbegin(); ch_it != str.rend(); ++ch_it)	
	{
		bool suffix_done = false;
		switch (*ch_it)
		{
		case 'u':
		case 'U':
			is_unsigned = true;
			++tail_count;
			break;
		case 'l':
		case 'L':
			is_long = true;
			++tail_count;
			break;
		default:
			suffix_done = true;
			break;
		}

		if(suffix_done) break;
	}
	
	// remove suffix for lexical casting.
	return str.substr(0, str.length() - tail_count);
}

string_ref split_real_literal_suffix(string_ref str, bool& is_single)
{
	is_single = false;

	auto ch_it = str.rbegin();
	if ( *ch_it == 'F' || *ch_it == 'f' )
	{
		is_single = true;
	}

	// remove suffix for lexical casting.
	return is_single ? str.substr(0, str.length()-1) : str;
}

BEGIN_NS_SASL_SEMANTIC();

class module_semantic_impl: public module_semantic
{
public:
	template <typename T, typename PoolT>
	static T* alloc_object( PoolT& p )
	{
		assert( sizeof(T) == p.get_requested_size() );
		T* ret = static_cast<T*>( p.malloc() );
		return ret;
	}

	module_semantic_impl()
		: node_semantic_pool_( sizeof(node_semantic) )
		, symbol_pool_( sizeof(symbol) )
	{
		pety_		= pety_t::create(this);
		root_symbol_= symbol::create_root(this);

		diag_chat_	= diag_chat::create();
		pety_->root_symbol(root_symbol_);
	}

	~module_semantic_impl()
	{
		clean_node_semantics();
		clean_symbols();
	}

	virtual symbol* root_symbol() const
	{
		return root_symbol_;
	}

	virtual program_ptr get_program() const
	{
		return root_node_;
	}

	virtual	void set_program(sasl::syntax_tree::program_ptr const& v)
	{
		root_node_ = v;
	}

	virtual pety_t* pety() const
	{
		return pety_.get();
	}

	virtual diag_chat_ptr diags() const
	{
		return diag_chat_;
	}

	virtual vector<symbol*> const&	global_vars() const
	{
		return global_vars_;
	}

	virtual vector<symbol*>& global_vars()
	{
		return global_vars_;
	}

	virtual vector<symbol*> const& functions() const
	{
		return functions_;
	}

	virtual vector<symbol*>& functions()
	{
		return functions_;
	}

	virtual vector<symbol*> const& intrinsics() const
	{
		return intrinsics_;
	}

	virtual vector<symbol*>& intrinsics()
	{
		return intrinsics_;
	}

	virtual bool is_modified(symbol* v) const
	{
		return modified_symbols_.count(v) > 0;
	}
			 
	virtual void modify(symbol* v)
	{
		modified_symbols_.insert(v);
	}

	virtual node_semantic* get_semantic(node const* v) const
	{
		assert(v);
		semantics_dict::const_iterator it = semantics_dict_.find(v);
		if( it == semantics_dict_.end() ){ 
			return NULL;
		}
		return it->second;
	}

	virtual node_semantic* create_semantic(node const* v)
	{
		assert(v);
		assert( !get_semantic(v) );
		node_semantic* ret = alloc_semantic();
		ret->associated_node( const_cast<node*>(v) );
		semantics_dict_.insert( make_pair(v, ret) );
		return ret;
	}

	virtual node_semantic* get_or_create_semantic(node const* v)
	{
		assert(v);
		node_semantic* ret = get_semantic(v);
		return ret ? ret : create_semantic(v);
	}

	virtual symbol* get_symbol(sasl::syntax_tree::node* v) const
	{
		symbols_dict::const_iterator it = symbols_dict_.find(v);

		if ( it != symbols_dict_.end() )
		{
			return it->second;
		}

		return NULL;
	}

	virtual symbol* alloc_symbol()
	{
		symbol* ret = alloc_object<symbol>(symbol_pool_);
		symbols_.push_back(ret);
		return ret;
	}

	virtual void link_symbol(node* v, symbol* sym)
	{
		// Only available for symbol create by this module.
		assert( sym->owner() == this );
		// symbol* ref_sym = get_symbol(v);
		assert( get_symbol(v) == NULL );	// v was not connected to symbol.

		node* old_assoc_node = sym->associated_node();
		
		if(old_assoc_node != NULL)
		{
			symbols_dict_.erase(old_assoc_node);	
		}

		if( v != NULL )
		{
			symbols_dict_.insert( make_pair(const_cast<node const*>(v), sym) );
		}

		sym->associated_node(v);
	}

	void hold_generated_node(sasl::syntax_tree::node_ptr const& v)
	{
		extra_nodes_.push_back(v);
	}

	salviar::languages get_language() const
	{
		return lang_;
	}

	void set_language(salviar::languages v)
	{
		lang_ = v;
	}
private:
	node_semantic* alloc_semantic()
	{
		node_semantic* ret = static_cast<node_semantic*>( node_semantic_pool_.malloc() );
		semantics_.push_back(ret);
		memset( ret, 0, sizeof(node_semantic) );
		ret->owner(this); 
		ret->tid(-1);
		return ret;
	}

	void clean_node_semantics()
	{
		for(vector<node_semantic*>::iterator it = semantics_.begin(); it != semantics_.end(); ++it )
		{
			(*it)->~node_semantic();
		}
	}

	void clean_symbols()
	{
		for(vector<symbol*>::iterator it = symbols_.begin(); it != symbols_.end(); ++it )
		{
			(*it)->~symbol();
		}
	}

	salviar::languages	lang_;
	pety_t_ptr			pety_;
	program_ptr			root_node_;
	symbol*				root_symbol_;
	diag_chat_ptr		diag_chat_;


	vector<node_ptr>	extra_nodes_;  // Hold nodes generated by semantic analyzer.

	vector<symbol*>		global_vars_;
	vector<symbol*>		functions_;
	vector<symbol*>		intrinsics_;

	boost::pool<>			node_semantic_pool_;
	boost::pool<>			symbol_pool_;
	vector<node_semantic*>	semantics_;
	vector<symbol*>			symbols_;
	typedef unordered_map<node const*, node_semantic*>	semantics_dict;
	typedef unordered_map<node const*, symbol*>			symbols_dict;
	semantics_dict	semantics_dict_;
	symbols_dict	symbols_dict_;

	unordered_set<symbol const*> modified_symbols_;
};

string const& node_semantic::function_name() const
{
	if( !function_name_ )
	{ 
		const_cast<node_semantic*>(this)->function_name_
			= new string();
	}
	return *function_name_;
}

void node_semantic::function_name( std::string const& v )
{
	if( !function_name_ )
	{ 
		function_name_ = new string(v);
	}
	else
	{
		*function_name_ = v;
	}
}

node_semantic::labeled_statement_array const&
	node_semantic::labeled_statements() const
{
	return const_cast<node_semantic*>(this)->labeled_statements();
}

node_semantic::labeled_statement_array& node_semantic::labeled_statements()
{
	if( !labeled_statements_ )
	{
		labeled_statements_ = new labeled_statement_array();
	}
	return *labeled_statements_;
}

void node_semantic::semantic_value( salviar::semantic_value const& v )
{
	if( !semantic_value_ )
	{
		semantic_value_ = new salviar::semantic_value(v);
	}
	else
	{
		*semantic_value_ = v;
	}
}

node_semantic::~node_semantic()
{
	if( semantic_value_ )
	{
		delete semantic_value_;
		semantic_value_ = NULL;
	}

	if( function_name_ )
	{
		delete function_name_;
		function_name_ = NULL;
	}

	if( labeled_statements_ )
	{
		delete labeled_statements_;
		labeled_statements_ = NULL;
	}

	if( string_constant_ )
	{
		delete string_constant_;
		string_constant_ = NULL;
	}
}

void node_semantic::const_value(string const& lit, literal_classifications lit_class)
{
	string_ref nosuffix_litstr;
	builtin_types value_btc(builtin_types::none);

	if (lit_class == literal_classifications::integer )
	{
		bool is_unsigned(false);
		bool is_long(false);
		nosuffix_litstr = split_integer_literal_suffix( lit, is_unsigned, is_long );
		if ( is_unsigned )
		{
			unsigned_constant_ = boost::lexical_cast<uint64_t>(nosuffix_litstr);
			value_btc = is_long ? builtin_types::_uint64 : builtin_types::_uint32;
		}
		else
		{
			signed_constant_ = boost::lexical_cast<int64_t>(nosuffix_litstr);
			value_btc = is_long ? builtin_types::_sint64 : builtin_types::_sint32;
		}
	}
	else if( lit_class == literal_classifications::real )
	{
		bool is_single(false);
		nosuffix_litstr = split_real_literal_suffix(lit, is_single);
		double_constant_ = boost::lexical_cast<double>(nosuffix_litstr);
		value_btc = is_single ? builtin_types::_float : builtin_types::_double;
	}
	else if( lit_class == literal_classifications::boolean )
	{
		signed_constant_ = (lit == "true" ? 1 : 0);
		value_btc = builtin_types::_boolean;
	}
	else if( lit_class == literal_classifications::character )
	{
		signed_constant_ = lit[0];
		value_btc = builtin_types::_sint8;
	}
	else if( lit_class == literal_classifications::string )
	{
		const_value(lit);
		value_btc = builtin_types::none;
	}

	tid_ = owner_->pety()->get(value_btc);
	proto_type_ = owner_->pety()->get_proto_by_builtin(value_btc);
}

std::string node_semantic::const_string() const
{
	return string_constant_ ? *string_constant_ : string();
}

void node_semantic::const_value( std::string const& v )
{
	if( string_constant_ == NULL )
	{
		string_constant_ = new string(v);
	}
	*string_constant_ = v;
	tid(-1);
}

salviar::semantic_value const& node_semantic::semantic_value_ref() const
{
	if( !semantic_value_ )
	{
		const_cast<node_semantic*>(this)->semantic_value_
			= new salviar::semantic_value();
	}
	return *semantic_value_;
}

builtin_types node_semantic::value_builtin_type() const
{
	return proto_type_ ? proto_type_->tycode : builtin_types::none;
}

void node_semantic::tid(int v)
{
	if( tid_ != v )
	{ 
		tid_ = v;
		proto_type_ = owner_->pety()->get_proto(tid_);
	}
}

node_semantic::node_semantic( node_semantic const& )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

node_semantic& node_semantic::operator=( node_semantic const& v )
{
	memcpy( this, &v, sizeof(node_semantic) );
	if(string_constant_)
	{
		string_constant_ = NULL;
		const_value( v.const_string() );
	}
	if(semantic_value_)
	{
		semantic_value_ = NULL;
		semantic_value( v.semantic_value_ref() );
	}
	if(function_name_)
	{
		function_name_ = NULL;
		function_name( v.function_name() );
	}
	return *this;
}

tynode* node_semantic::ty_proto() const
{
	return proto_type_;
}

void node_semantic::ty_proto(tynode* ty, symbol* scope)
{
	tid( owner_->pety()->get(ty, scope) );
}

void node_semantic::internal_tid( int v, tynode* proto )
{
	tid_		= v;
	proto_type_	= proto;
}

module_semantic_ptr module_semantic::create()
{
	return make_shared<module_semantic_impl>();
}

END_NS_SASL_SEMANTIC();