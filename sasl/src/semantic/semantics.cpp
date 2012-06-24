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
#include <boost/lexical_cast.hpp>
#include <eflib/include/platform/boost_end.h>

EFLIB_USING_SHARED_PTR(sasl::syntax_tree, program);
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, node);
EFLIB_USING_SHARED_PTR(sasl::common, diag_chat);
using boost::unordered_map;
using boost::shared_ptr;
using std::vector;
using std::string;
using std::make_pair;

string split_integer_literal_suffix( string const& str, bool& is_unsigned, bool& is_long ){
	is_unsigned = false;
	is_long = false;

	string::const_reverse_iterator ch_it = str.rbegin();
	char ch[2] = {'\0', '\0'};
	ch[0] = *ch_it;
	++ch_it;
	ch[1] = (ch_it == str.rend() ? '\0' : *ch_it);

	int tail_count = 0;
	for ( int i = 0; i < 2; ++i ){
		switch (ch[i]){
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
			// do nothing
			break;
		}
	}
	
	// remove suffix for lexical casting.
	return string( str.begin(), str.end()-tail_count );
}

string split_real_literal_suffix(string const& str, bool& is_single)
{
	is_single = false;

	string::const_reverse_iterator ch_it = str.rbegin();
	if ( *ch_it == 'F' || *ch_it == 'f' ){
		is_single = true;
	}

	// remove suffix for lexical casting.
	return is_single ? string(str.begin(), str.end()-1 ) : str;
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
		printf("new module_semantic_impl @ 0x%08X\n", this);
		
		pety_		= pety_t::create(this);
		printf("Pety @ 0x%08X\n", pety_.get());
		
		root_symbol_= symbol::create_root(this);
		printf("Root symbol @ 0x%08X\n", root_symbol_);

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

	virtual node_semantic* get_semantic(node const* v) const
	{
		unordered_map<node const*, node_semantic*>::const_iterator it
			= semantics_dict_.find(v);
		if( it == semantics_dict_.end() ){ return NULL; }
		return it->second;
	}

	virtual node_semantic* get_or_create_semantic(node const* v)
	{
		node_semantic* ret = get_semantic(v);
		if( ret == NULL )
		{
			ret = alloc_semantic();
			semantics_dict_.insert( make_pair(v, ret) );
		}
		return ret;
	}

	virtual symbol* get_symbol(sasl::syntax_tree::node* v) const
	{
		unordered_map<node const*, symbol*>::const_iterator
			it = symbols_dict_.find(v);

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
		symbol* ref_sym = get_symbol(v);
		assert( get_symbol(v) == NULL );	// v was not connected to symbol.

		node* old_assoc_node = sym->associated_node();
		if(old_assoc_node != NULL) {
			symbols_dict_.erase(old_assoc_node);	
		}

		if( v != NULL ) {
			symbols_dict_.insert( make_pair(v, sym) );
		}

		sym->associated_node(v);
	}

	void hold_node(sasl::syntax_tree::node_ptr const& v)
	{
		extra_nodes_.push_back(v);
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

	pety_t_ptr		pety_;
	program_ptr		root_node_;
	symbol*			root_symbol_;
	diag_chat_ptr	diag_chat_;


	vector< shared_ptr<node> > extra_nodes_;  // Hold nodes generated by semantic analyzer.

	vector<symbol*> global_vars_;
	vector<symbol*> functions_;
	vector<symbol*> intrinsics_;

	boost::pool<>			node_semantic_pool_;
	boost::pool<>			symbol_pool_;
	vector<node_semantic*>	semantics_;
	vector<symbol*>			symbols_;
	unordered_map<node const*, node_semantic*>	semantics_dict_;
	unordered_map<node const*, symbol*>			symbols_dict_;
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
	string nosuffix_litstr;
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
	return proto_type_ ? builtin_types::none : proto_type_->tycode;
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

module_semantic_ptr module_semantic::create()
{
	return module_semantic_ptr( new module_semantic_impl() );
}

int32_t swizzle_field_name_to_id( char ch ){
	switch( ch ){
	case 'x':
	case 'r':
		return 1;
	case 'y':
	case 'g':
		return 2;
	case 'z':
	case 'b':
		return 3;
	case 'w':
	case 'a':
		return 4;
	}
	return 0;
}

int32_t encode_swizzle( char _1st, char _2nd, char _3rd, char _4th ){
	int32_t swz = 0;

	if( _1st == 0 ){
		return 0;
	} else {
		assert( swizzle_field_name_to_id(_1st) );
		swz = swizzle_field_name_to_id(_1st);
	}

	if( _2nd == 0){
		return swz;
	} else {
		assert( swizzle_field_name_to_id(_2nd) );
		swz &= ( _2nd << 8);
	}

	if( _3rd == 0){
		return swz;
	} else {
		assert( swizzle_field_name_to_id(_3rd) );
		swz &= ( _3rd << 16);
	}

	assert( swizzle_field_name_to_id(_4th) );
	swz &= ( _4th << 24);

	return swz;
}

int32_t encode_swizzle( int& dest_size, int& min_src_size, char const* masks ){
	min_src_size = 0;
	dest_size = 0;
	int32_t swz = 0;
	for( char const* p = &masks[0];;++p){
		if( *p ){
			int32_t field_swz = swizzle_field_name_to_id(*p);
			assert( field_swz );
			swz += ( field_swz << (dest_size * 8) );
			if( field_swz > min_src_size ){
				min_src_size = field_swz;
			}
			++dest_size;
		} else {
			break;
		}
	}

	return swz;
}

int32_t encode_sized_swizzle( int size )
{
	int32_t swz = 0;
	for( int32_t i = 1; i <= size; ++i ){
		swz &= ( i << (i-1) * 8 );
	}
	return swz;
}

END_NS_SASL_SEMANTIC();