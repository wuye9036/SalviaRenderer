#include <sasl/include/semantic/pety.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/utility.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/enums/builtin_types.h>
#include <sasl/enums/operators.h>
#include <sasl/enums/default_hasher.h>
#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/utility/polymorphic_cast.h>

#include <eflib/include/memory/atomic.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

using namespace sasl::syntax_tree;
using namespace sasl::utility;
using eflib::polymorphic_cast;
using eflib::fixed_string;
using eflib::spinlock;
using eflib::scoped_spin_locker;
using boost::atomic;
using boost::make_shared;
using boost::shared_ptr; // prevent conflicting with std::tr1.
using boost::dynamic_pointer_cast;
using boost::unordered_map;
using std::make_pair;
using std::vector;
using std::string;

BEGIN_NS_SASL_SEMANTIC();

// --------------------- Data Types -------------------------

class type_item{
public:
	typedef int type_item::*id_ptr_t;

	type_item::type_item()
		: u_qual(-1), a_qual(-1), ty_sem(NULL) {}

	tynode_ptr		stored;
	node_semantic*	ty_sem;

	tid_t			u_qual;
	tid_t			a_qual;
};

// Internal functions
node_semantic*	assign_tid_to_node	(module_semantic* msem, tynode* node, tynode* proto, tid_t tid);
tynode_ptr		duplicate_tynode	(tynode_ptr const& typespec);
fixed_string	builtin_type_name	(builtin_types btc);

void			init_builtin_short_name();
void			append_mangling		(string&, tynode*);
void			append_mangling		(string&, builtin_types btc, bool as_comp = false);

class pety_impl: public pety_t{
public:
	friend class pety_t;
	void root_symbol( symbol* sym )
	{
		root_symbol_ = sym;
	}

	// From builtin code to tid.
	tid_t pety_impl::get(builtin_types const& btc)
	{
		// If it is existed, return it.
		bt_dict::iterator it = bt_dict_.find(btc);
		if( it != bt_dict_.end() )
		{
			return it->second;
		}

		// Otherwise create a new type and push into type manager.
		tid_t btc_tid = -1;
		register_builtin_type(btc, NULL, &btc_tid);
		bt_dict_.insert( make_pair(btc, btc_tid) );

		return btc_tid;
	}
	tid_t pety_impl::get(tynode* v, symbol* scope)
	{
		return get_impl(v, scope, true);
	}
	tid_t get_array(tid_t elem_type, size_t dimension)
	{
		tid_t ret_tid = elem_type;
		for(size_t i = 1; i < dimension; ++i)
		{
			if( ret_tid == -1 ) break;
			ret_tid = type_items_[ret_tid].a_qual;
		}
		return ret_tid;
	}
	tid_t	get_function_type(vector<tid_t> const& fn_tids)
	{
		function_dict::iterator iter = fn_dict_.find(fn_tids);
		if( iter != fn_dict_.end() )
		{
			return iter->second;
		}

		// Construct function type prototype.
		shared_ptr<function_type> fn_proto = create_node<function_type>( token_t_ptr(), token_t_ptr() );
		fn_proto->result_type = get_proto(fn_tids[0])->as_handle<tynode>();
		fn_proto->param_types.reserve(4);
		for(size_t i_param = 1; i_param < fn_tids.size(); ++i_param)
		{
			fn_proto->param_types.push_back( get_proto(fn_tids[i_param])->as_handle<tynode>() );
		}

		// Register function type prototype.
		tid_t ret(-1);
		register_proto_tynode(fn_proto, NULL, &ret);

		// Update cache.
		param_tids_cache_.insert(
			make_pair(ret, vector<tid_t>( fn_tids.begin()+1, fn_tids.end() ) )
			);
		return ret;
	}
	// Get prototype or node semantic from tid or builtin_type
	tynode* get_proto(tid_t id)
	{
		return id < 0 ? NULL : type_items_[id].stored.get();
	}
	tynode* get_proto_by_builtin(builtin_types bt)
	{
		return get_proto( get(bt) );
	}
	void	get2(tid_t tid, tynode** out_tyn, node_semantic** out_sem)
	{
		if(out_tyn) { *out_tyn = type_items_[tid].stored.get(); }
		if(out_sem) { *out_sem = type_items_[tid].ty_sem; }
	}
	void	get2(builtin_types btc, tynode** out_tyn, node_semantic** out_sem)
	{
		tid_t btc_tid = get(btc);
		get2(btc_tid, out_tyn, out_sem);
	}

	fixed_string mangle(fixed_string const& name, tid_t tid)
	{
		init_builtin_short_name();

		// start char
		fixed_string ret;
		string& mangled_name( ret.mutable_raw_string() );
		mangled_name.reserve(64);
		mangled_name = "M";

		mangled_name.append( name.raw_string() );
		mangled_name.append("@@");
		append_params_mangling(mangled_name, param_tids_cache_[tid]);

		// TODO calling convention

		return ret;
	}

	fixed_string operator_name(operators const& op)
	{
		opname_cache::iterator it = opname_cache_.find(op);
		if( it != opname_cache_.end() )
		{
			return it->second;
		}
		fixed_string opname("0");
		opname.mutable_raw_string().append( op.name() );
		opname_cache_.insert( make_pair(op, opname) );
		return opname;
	}

	~pety_impl(){}

private:
	void register_proto_tynode(tynode_ptr const& proto_node, node_semantic** out_sem, tid_t* out_tid)
	{
		// add to pool and allocate an id
		tid_t proto_tid = static_cast<tid_t>( type_items_.size() );

		type_item proto_item;
		proto_item.stored = proto_node;
		proto_item.ty_sem = assign_tid_to_node(owner_, proto_node.get(), proto_node.get(), proto_tid);

		type_items_.push_back(proto_item);
		tynode_dict_.insert( make_pair(proto_node.get(), proto_tid) );

		if(out_sem) { *out_sem = proto_item.ty_sem; }
		if(out_tid) { *out_tid = proto_tid; }
	}

	void register_tynode(tynode* tyn, bool attach_tid_to_input, node_semantic** out_sem, tid_t* out_tid )
	{
		tynode_ptr dup_node = duplicate_tynode( tyn->as_handle<tynode>() );
		tid_t tid = -1;
		register_proto_tynode(dup_node, out_sem, &tid);
		if(out_tid) { *out_tid = tid; }

		if(attach_tid_to_input)
		{
			assign_tid_to_node(owner_, tyn, dup_node.get(), tid);
		}
	}

	/// Add builtin type to pety.
	void register_builtin_type(builtin_types btc, node_semantic** out_sem, tid_t* out_tid)
	{
		std::string name = builtin_type_name(btc);

#ifdef EFLIB_DEBUG
		symbol* sym = root_symbol_->find(name);
		assert(!sym);
#endif

		tynode_ptr tyn = create_builtin_type(btc);
		node_semantic* sem = NULL;
		register_proto_tynode(tyn, &sem, out_tid);
		if(out_sem) { *out_sem = sem; }
		assert( sem->associated_node() == tyn.get() );
		root_symbol_->add_named_child( name, tyn.get() );
	}

	tid_t get_impl(tynode* v, symbol* scope, bool attach_tid_to_input);

	// Append mangling to name. Mangling cached could be updated.
	void append_params_mangling(string& name, vector<tid_t> const& param_tids)
	{
		if( param_tids.empty() )
		{
			return;
		}

		mangling_cache::iterator it = mangling_cache_.find(param_tids);
		if (it != mangling_cache_.end() )
		{
			name.append( it->second.raw_string() );
		}
		else
		{
			fixed_string ret;

			string& mangling( ret.mutable_raw_string() );
			tynode* param0_type = type_items_[ param_tids[0] ].stored.get();
			append_mangling(mangling, param0_type);
			mangling.append("@@");
			vector<tid_t> tails( param_tids.begin() + 1, param_tids.end() );
			append_params_mangling(mangling, tails);

			mangling_cache_.insert( make_pair(param_tids, ret) );
			name.append(mangling);
		}
	}

	typedef unordered_map<builtin_types, tid_t>			bt_dict;
	typedef unordered_map<tynode*,		 tid_t>			tynode_dict;
	typedef unordered_map<vector<tid_t>, tid_t>			function_dict;
	typedef unordered_map<vector<tid_t>, fixed_string>	mangling_cache;
	typedef	unordered_map< tid_t, vector<tid_t> >		param_tids_cache;
	typedef unordered_map<operators, fixed_string>		opname_cache;

	bt_dict				bt_dict_;
	tynode_dict			tynode_dict_;
	function_dict		fn_dict_;
	vector<type_item>	type_items_;
	symbol*				root_symbol_;
	module_semantic*	owner_;
	mangling_cache		mangling_cache_;
	param_tids_cache	param_tids_cache_;
	opname_cache		opname_cache_;
};

shared_ptr<pety_t> pety_t::create(module_semantic* owner)
{
	pety_impl* ret = new pety_impl();
	ret->owner_ = owner;
	return shared_ptr<pety_t>(ret);
}

// ---------------------------------------------------------
node_semantic* assign_tid_to_node( module_semantic* msem, tynode* node, tynode* proto, tid_t tid )
{
	node_semantic* ret = msem->create_semantic(node);
	ret->internal_tid(tid, proto);
	return ret;
}

tid_t get_node_tid( unordered_map<tynode*, tid_t> const& dict, tynode* nd )
{
	if( !nd ) { return -1; }
	unordered_map<tynode*, tid_t>::const_iterator it = dict.find(nd);
	if( it == dict.end() ) { return -1; }
	return it->second;
}

tid_t get_symbol_tid( unordered_map<tynode*, tid_t> const& dict, symbol* sym ){
	if( !sym ) { return -1; }
	return get_node_tid( dict, polymorphic_cast<tynode*>( sym->associated_node() ) );
}

unordered_map<builtin_types, fixed_string> builtins_name;
fixed_string builtin_type_name( builtin_types btc ){
	unordered_map<builtin_types, fixed_string>::iterator it = builtins_name.find(btc);
	if( it != builtins_name.end() )
	{
		return it->second;
	}

	std::string ret;
	if( is_vector(btc) )
	{
		ret = 
			( boost::format("%1%_%2%") 
			% builtin_type_name( scalar_of(btc) )
			% vector_size( btc )
			).str();

	}
	else if( is_matrix(btc) )
	{
		ret =
			( boost::format("%1%_%2%x%3%") 
			% builtin_type_name( scalar_of(btc) )
			% vector_size( btc )
			% vector_count( btc )
			).str();

	}
	else
	{
		ret = ( boost::format("0%1%") % btc.name() ).str();
	}

	builtins_name.insert( make_pair(btc, ret) );
	return ret;
}

//	description:
//		remove qualifier from type.
//	return:
//		means does the peeling was executed actually.
//		If the src is unqualified type, it returns 'false',
//		naked was assigned from src, and qual return a null ptr.
bool peel_qualifier(tynode* src, tynode*& naked, type_item::id_ptr_t& qual)
{
	if( src->is_uniform() )
	{
		naked = duplicate( src->as_handle() )->as_handle<tynode>().get();
		naked->qual = type_qualifiers::none;
		qual = &type_item::u_qual;
		return true;
	}

	if( src->is_array() )
	{
		array_type_ptr derived_naked = duplicate( src->as_handle() )->as_handle<array_type>();
		derived_naked->array_lens.pop_back();
		naked = derived_naked->array_lens.empty() ? derived_naked->elem_type.get() : derived_naked.get();
		qual = &type_item::a_qual;
		return true;
	}

	naked = src;
	qual = NULL;
	return false;
}

tynode_ptr duplicate_tynode(tynode_ptr const& typespec){
	if( typespec->is_struct() ){
		// NOTE:
		//	Clear declarations of duplicated since they must be filled by struct visitor.
		shared_ptr<struct_type> ret_struct = duplicate(typespec)->as_handle<struct_type>();
		ret_struct->decls.clear();
		return ret_struct;
	} else {
		return duplicate(typespec)->as_handle<tynode>();
	}
}

fixed_string name_of_unqualified_type(module_semantic* /*sem*/, tynode* typespec)
{
	// Only build in, struct and function are potential unqualified type.
	// Array type is qualified type.

	node_ids node_cls = typespec->node_class();

	if(node_cls == node_ids::alias_type)
	{
		return polymorphic_cast<alias_type*>(typespec)->alias->str;
	}
	else if(node_cls == node_ids::builtin_type)
	{
		return builtin_type_name(typespec->tycode);
	}
	else if(node_cls == node_ids::function_full_def)
	{
		assert(!"Function full def is not supported.");
		return fixed_string();
		// return mangle( sem, polymorphic_cast<function_full_def*>(typespec) );
	}
	else if(node_cls == node_ids::struct_type)
	{
		return polymorphic_cast<struct_type*>(typespec)->name->str;
	}
	else if(node_cls == node_ids::function_type)
	{
		return fixed_string();
	}

	assert( !"Type type code is unrecognized!" );
	return fixed_string();
}

tid_t pety_impl::get_impl(tynode* v, symbol* scope, bool attach_tid_to_input)
{
	// Return id if existed.
	tid_t ret = get_node_tid(tynode_dict_, v);
	if( ret != -1 ){ return ret; }

	// otherwise process the node for getting right id;
	type_item::id_ptr_t qual;
	tynode* inner_type;

	if( peel_qualifier(v, inner_type, qual) )
	{
		tid_t decoratee_id = get(inner_type, scope);
		if( decoratee_id == -1 ) { return -1; }
		if( type_items_[decoratee_id].*qual >= 0 ){
			// The qualified node is in items yet.
			return type_items_[decoratee_id].*qual;
		} else {
			// else allocate an new node.
			register_tynode(v, attach_tid_to_input, NULL, &ret);
			return ret;
		}
	}
	else
	{
		// Here type specifier is a unqualified type.
		// Look up the name of type in symbol.
		// If it did not exist, throw an error or add it into symbol(as an swallow copy).
		fixed_string name = name_of_unqualified_type(scope->owner(), v);
		symbol* sym = name.empty() ? NULL : scope->find(name);
		if(sym)
		{
			return get_symbol_tid(tynode_dict_, sym);
		}
		else
		{
			node_ids node_cls = v->node_class();
			if(node_cls == node_ids::function_type)
			{
				function_type* fnty = polymorphic_cast<function_type*>(v);
				vector<tid_t> tids;
				tid_t tid = -1;
				tid = tynode_dict_.at(fnty->result_type.get());
				assert(tid != -1);
				tids.push_back(tid);
				for(size_t i = 0; i < fnty->param_types.size(); ++i)
				{
					tid = tynode_dict_.at(fnty->param_types[i].get());
					assert(tid != -1);
					tids.push_back(tid);
				}
				function_dict::iterator iter = fn_dict_.find(tids);
				if(iter != fn_dict_.end())
				{
					return iter->second;
				}
			}

			if(node_cls  == node_ids::alias_type)
			{
				return -1;
			}

			tid_t tid = -1;
			register_tynode(v, attach_tid_to_input, NULL, &tid);

			if( !name.empty() )
			{
				scope->add_named_child( name, type_items_[tid].stored.get() );
			}

			return tid;
		}
	}
}

// ---------------------Name Mangling---------------------------

// lookup table for translating enumerations to string.
string mangling_tag("M");
unordered_map<builtin_types, string> btc_decorators;
spinlock builtin_shorten_mutex;
bool builtin_shorten_initialized(false);
void init_builtin_short_name()
{
	scoped_spin_locker lck(builtin_shorten_mutex);

	if(builtin_shorten_initialized) { return; }

	boost::assign::insert( btc_decorators )
		( builtin_types::_void, "O" )
		( builtin_types::_boolean, "B" )
		( builtin_types::_sint8, "S1" )
		( builtin_types::_sint16, "S2" )
		( builtin_types::_sint32, "S4" )
		( builtin_types::_sint64, "S8" )
		( builtin_types::_uint8, "U1" )
		( builtin_types::_uint16, "U2" )
		( builtin_types::_uint32, "U4" )
		( builtin_types::_uint64, "U8" )
		( builtin_types::_float, "F" )
		( builtin_types::_double, "D" )
		;

	builtin_shorten_initialized = true;
}

void append_mangling(string& str, builtin_types btc, bool as_comp)
{
	if ( is_scalar( btc ) ) {
		if ( !as_comp ){
			// if it is not a component of a vector or matrix,
			// add a lead char 'B' since is a BaseTypeName of a builtin scalar type.
			str.insert(str.end(), 'B');
		}
		str.append( btc_decorators[btc] );
	} else if( is_vector( btc ) ) {
		str.append("V");
		char vector_len_ch = ( '0' + static_cast<char>( vector_size(btc) ) );
		str.append(1, vector_len_ch);
		append_mangling( str, scalar_of(btc), true );
	} else if ( is_matrix(btc) ) {
		str.append("M");
		char matrix_len_buf[3] = {
			'0' + static_cast<char>( vector_size(btc) )
			, '0' + static_cast<char>( vector_count(btc) )
			, 0
		};
		str.append(matrix_len_buf);
		append_mangling(str, scalar_of(btc), true);
	}
}

void append_mangling(std::string& str, type_qualifiers qual){
	if ( qual.included( type_qualifiers::_uniform ) ){
		str.append("U");
	}
	str.append("Q");
}

void append_mangling(std::string& str, struct_type* stype){
	str.append("S");
	str.append( stype->name->str );
}

void append_mangling(std::string& str, array_type* atype){
	for ( size_t i_dim = 0; i_dim < atype->array_lens.size(); ++i_dim ){
		str.append("A");
		// str.append( atype->array_lens[i_dim] );
		// if ( i_dim < atype->array_lens.size() - 1 ){
		//	str.append("Q");
		// }
	}
	append_mangling( str, atype->elem_type.get() );
}

void append_mangling(std::string& str, tynode* typespec){
	append_mangling(str, typespec->qual);
	node_ids node_cls = typespec->node_class();
	if (node_cls == node_ids::builtin_type)
	{
		append_mangling( str, typespec->tycode );
	}
	else if ( typespec->node_class() == node_ids::struct_type )
	{
		append_mangling( str, polymorphic_cast<struct_type*>(typespec) );
	}
	else if( typespec->node_class() == node_ids::array_type )
	{
		append_mangling( str, polymorphic_cast<array_type*>(typespec) );
	}
	else
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}

END_NS_SASL_SEMANTIC();
