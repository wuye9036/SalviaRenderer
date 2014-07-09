#include <sasl/include/semantic/reflection_impl.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/host/utility.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/math/math.h>

using namespace sasl::utility;

EFLIB_USING_SHARED_PTR(sasl::syntax_tree, array_type);
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, tynode);

using salviar::sv_usage;
using salviar::su_none;
using salviar::su_stream_in;
using salviar::su_stream_out;
using salviar::su_buffer_in;
using salviar::su_buffer_out;
using salviar::sv_usage_count;
using salviar::sv_position;
using salviar::lang_pixel_shader;
using salviar::lang_vertex_shader;
using salviar::PACKAGE_ELEMENT_COUNT;
using salviar::SIMD_ELEMENT_COUNT;

using salviar::sv_layout;

using eflib::ceil_to_pow2;
using eflib::fixed_string;

using boost::addressof;
using boost::shared_ptr;

using std::vector;
using std::make_pair;

BEGIN_NS_SASL_SEMANTIC();

using namespace sasl::utility::ops;

reflection_impl::reflection_impl()
	: module_sem_(NULL), entry_point_(NULL), position_output_(NULL)
{
	memset( counts_,  0, sizeof(counts_)  );
	memset( offsets_, 0, sizeof(offsets_) );
}

void reflection_impl::module( shared_ptr<module_semantic> const& v )
{
	module_sem_ = v.get();
}

bool reflection_impl::is_module( shared_ptr<module_semantic> const& v ) const
{
	return module_sem_ == v.get();
}

void reflection_impl::entry(symbol* v)
{
	entry_point_ = v;
	entry_point_name_ = entry_point_->mangled_name();
}

bool reflection_impl::is_entry(symbol* v) const
{
	return entry_point_ == v;
}

fixed_string reflection_impl::entry_name() const
{
	return entry_point_name_;
}

bool reflection_impl::add_input_semantic( salviar::semantic_value const& sem, builtin_types btc, bool is_stream )
{
	vector<salviar::semantic_value>::iterator it =
		std::lower_bound(semantic_inputs_.begin(), semantic_inputs_.end(), sem);

	if(it != semantic_inputs_.end() && *it == sem)
	{
		return false;
	}

	sv_layout* si = alloc_input_layout( sem );
	si->value_type		= to_lvt( btc );
	si->agg_type			= salviar::aggt_none;
	si->internal_type	= -1;
	si->usage			= is_stream ? su_stream_in : su_buffer_in;
	si->sv				= sem;

	++counts_[si->usage];

	semantic_inputs_.insert(it, sem);

	return true;
}

bool reflection_impl::add_output_semantic(salviar::semantic_value const& sem, builtin_types btc, bool is_stream)
{
	// semantic_outputs_ is ordered array.
	vector<salviar::semantic_value>::iterator it = std::lower_bound(semantic_outputs_.begin(), semantic_outputs_.end(), sem);
	if(it != semantic_outputs_.end() && *it == sem)
	{
		return false;
	}

	sv_layout* si = alloc_output_layout(sem);
	si->value_type		= to_lvt(btc);
	si->agg_type			= salviar::aggt_none;
	si->internal_type	= -1;
	si->usage			= is_stream ? su_stream_out : su_buffer_out;
	si->sv				= sem;

	++counts_[si->usage];

	if(si->sv.get_system_value() == sv_position)
	{
		position_output_ = si;
	}

	semantic_outputs_.insert(it, sem);

	return true;
}

void reflection_impl::add_global_var(symbol* v, tynode_ptr tyn)
{
	uniform_inputs_.push_back(v);

	sv_layout* si = alloc_input_layout(v);
	
	if( tyn->is_builtin() )
	{
		si->value_type = to_lvt(tyn->tycode);
	}
	else if( tyn->is_array() )
	{
		si->value_type	= to_lvt(tyn->as_handle<array_type>()->elem_type->tycode);
		assert(si->value_type != salviar::lvt_none);
		si->agg_type	= salviar::aggt_array;
	} 
	else
	{
		assert(false);
	}

	si->internal_type = module_sem_->get_semantic(tyn)->tid();
	si->usage = su_buffer_in;
	++counts_[si->usage];

	name_layouts_.insert( make_pair(v->unmangled_name(), si) );
}

sv_layout* reflection_impl::input_sv_layout( salviar::semantic_value const& sem ) const
{
	semantic_layout_dict::const_iterator it = semantic_input_layouts_.find( sem );
	if ( it == semantic_input_layouts_.end() )
	{
		return NULL;
	}
	return const_cast<sv_layout*>( addressof(it->second) );
}

sv_layout* reflection_impl::alloc_input_layout( salviar::semantic_value const& sem ){
	return addressof( semantic_input_layouts_[sem] );
}

sv_layout* reflection_impl::input_sv_layout( symbol* v ) const
{
	symbol_layout_dict::const_iterator it = uniform_input_layouts_.find(v);
	if ( it == uniform_input_layouts_.end() )
	{
		return NULL;
	}
	return const_cast<sv_layout*>( addressof( it->second ) );
}

sv_layout* reflection_impl::input_sv_layout(fixed_string const& name) const
{
	name_layout_dict::const_iterator it = name_layouts_.find(name);
	if( it == name_layouts_.end() )
	{
		return NULL;
	}
	return it->second;
}
sv_layout* reflection_impl::alloc_input_layout( symbol* v )
{
	return addressof( uniform_input_layouts_[v] );
}

sv_layout* reflection_impl::output_sv_layout( salviar::semantic_value const& sem ) const{
	semantic_layout_dict::const_iterator it = semantic_output_layouts_.find( sem );
	if ( it == semantic_output_layouts_.end() ){
		return NULL;
	}
	return const_cast<sv_layout*>( addressof( it->second ) );
}

size_t reflection_impl::total_size( sv_usage st ) const{
	return offsets_[st];
}

sv_layout* reflection_impl::alloc_output_layout( salviar::semantic_value const& sem )
{
	return addressof(semantic_output_layouts_[sem]);
}

std::vector<sv_layout*> reflection_impl::layouts(sv_usage usage) const{
	std::vector<sv_layout*> ret;

	// Process output
	if(usage == su_buffer_out || usage == su_stream_out){
		for(salviar::semantic_value const& sem: semantic_outputs_)
		{
			sv_layout* svl = output_sv_layout( sem );
			if ( svl->usage == usage ){
				ret.push_back( svl );
			}
		}
		return ret;
	}

	// Process input
	for (size_t index = 0; index < semantic_inputs_.size(); ++index)
	{
		sv_layout* svl = input_sv_layout( semantic_inputs_[index] );
		if (svl->usage == usage)
		{
			ret.push_back(svl);
		}
	}

	if (usage == su_buffer_in)
	{
		for(symbol* sym: uniform_inputs_)
		{
			ret.push_back( const_cast<sv_layout*>( addressof(uniform_input_layouts_.find(sym)->second) ) );
		}
	}

	return ret;
}

size_t reflection_impl::layouts_count(sv_usage usage) const
{
	return counts_[usage];
}

void reflection_impl::update_size(size_t sz, salviar::sv_usage usage)
{
	offsets_[usage] = static_cast<int>(sz);
}

bool reflection_impl::has_position_output() const
{
	return position_output_ != NULL;
}

salviar::languages reflection_impl::get_language() const
{
	return module_sem_->get_language();
}

END_NS_SASL_SEMANTIC();