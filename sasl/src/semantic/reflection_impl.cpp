#include <sasl/include/semantic/reflection_impl.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/host/utility.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/math/math.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

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
	: module_sem_(NULL), entry_point_(NULL), lang(salviar::lang_none)
{
	memset( counts, 0, sizeof(counts) );
	memset( offsets, 0, sizeof(offsets) );
}

void reflection_impl::module( shared_ptr<module_semantic> const& v ){
	module_sem_ = v.get();
}

bool reflection_impl::is_module( shared_ptr<module_semantic> const& v ) const{
	return module_sem_ == v.get();
}

void reflection_impl::entry( symbol* v ){
	entry_point_ = v;
	entry_point_name_ = entry_point_->mangled_name();
}

bool reflection_impl::is_entry( symbol* v ) const{
	return entry_point_ == v;
}

fixed_string reflection_impl::entry_name() const{
	return entry_point_name_;
}

bool reflection_impl::add_input_semantic( salviar::semantic_value const& sem, builtin_types btc, bool is_stream )
{
	vector<salviar::semantic_value>::iterator it = std::lower_bound( sems_in.begin(), sems_in.end(), sem );
	if( it != sems_in.end() ){
		if( *it == sem ){
			sv_layout* si = input_sv_layout(sem);
			assert(si);
			if( si->value_type == btc || si->value_type == builtin_types::none ){
				si->value_type = to_lvt(btc);
				si->agg_type = salviar::aggt_none;
				si->internal_type = -1;
				return true;
			}
			return false;
		}
	}

	sv_layout* si = alloc_input_storage( sem );
	si->value_type = to_lvt( btc );
	si->agg_type = salviar::aggt_none;
	si->internal_type = -1;
	si->usage = is_stream ? su_stream_in : su_buffer_in;
	si->sv = sem;
	sems_in.insert( it, sem );
	return true;
}

bool reflection_impl::add_output_semantic( salviar::semantic_value const& sem, builtin_types btc, bool is_stream  ){
	vector<salviar::semantic_value>::iterator it = std::lower_bound( sems_out.begin(), sems_out.end(), sem );
	if( it != sems_out.end() ){
		if( *it == sem ){
			sv_layout* si = alloc_output_storage( sem );
			if( si->value_type != btc && si->value_type == builtin_types::none ){
				si->value_type = to_lvt( btc );
				si->agg_type = salviar::aggt_none;
				si->internal_type = -1;
				return true;
			}
			return false;
		}
	}

	sv_layout* si = alloc_output_storage( sem );
	si->value_type = to_lvt(btc);
	si->agg_type = salviar::aggt_none;
	si->internal_type = -1;
	si->usage = is_stream ? su_stream_out : su_buffer_out;
	si->sv = sem;
	sems_out.insert( it, sem );
	return true;
}

void reflection_impl::add_global_var(symbol* v, tynode_ptr tyn)
{
	syms_in.push_back( v );

	sv_layout* si = alloc_input_storage( v );
	
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

	name_storages.insert( make_pair(v->unmangled_name(), si) );
}

sv_layout* reflection_impl::input_sv_layout( salviar::semantic_value const& sem ) const {
	sem_storages_t::const_iterator it = semin_storages.find( sem );
	if ( it == semin_storages.end() ){
		return NULL;
	}
	return const_cast<sv_layout*>( addressof( it->second ) );
}

sv_layout* reflection_impl::alloc_input_storage( salviar::semantic_value const& sem ){
	return addressof( semin_storages[sem] );
}

sv_layout* reflection_impl::input_sv_layout( symbol* v ) const {
	sym_storages_t::const_iterator it = symin_storages.find(v);
	if ( it == symin_storages.end() ){
		return NULL;
	}
	return const_cast<sv_layout*>( addressof( it->second ) );
}

sv_layout* reflection_impl::input_sv_layout(fixed_string const& name ) const
{
	name_storages_t::const_iterator it = name_storages.find( name );
	if( it == name_storages.end() ){
		return NULL;
	}
	return it->second;
}
sv_layout* reflection_impl::alloc_input_storage( symbol* v ){
	return addressof( symin_storages[v] );
}

sv_layout* reflection_impl::output_sv_layout( salviar::semantic_value const& sem ) const{
	sem_storages_t::const_iterator it = semout_storages.find( sem );
	if ( it == semout_storages.end() ){
		return NULL;
	}
	return const_cast<sv_layout*>( addressof( it->second ) );
}

size_t reflection_impl::total_size( sv_usage st ) const{
	return offsets[st];
}

sv_layout* reflection_impl::alloc_output_storage( salviar::semantic_value const& sem ){
	return addressof( semout_storages[sem] );
}

std::vector<sv_layout*> reflection_impl::layouts( sv_usage st ) const{
	std::vector<sv_layout*> ret;

	// Process output
	if( st == su_buffer_out || st == su_stream_out ){
		BOOST_FOREACH( salviar::semantic_value const& sem, sems_out ){
			sv_layout* svl = output_sv_layout( sem );
			if ( svl->usage == st ){
				ret.push_back( svl );
			}
		}
		return ret;
	}

	// Process input
	for ( size_t index = 0; index < sems_in.size(); ++index ){
		sv_layout* svl = input_sv_layout( sems_in[index] );
		if ( svl->usage == st ){
			ret.push_back( svl );
		}
	}

	if ( st == su_buffer_in ){
		BOOST_FOREACH( symbol* sym, syms_in ){
			ret.push_back( const_cast<sv_layout*>( addressof( symin_storages.find(sym)->second ) ) );
		}
	}

	return ret;
}

void reflection_impl::update_size( size_t sz, salviar::sv_usage usage )
{
	offsets[usage] = sz;
}

END_NS_SASL_SEMANTIC();