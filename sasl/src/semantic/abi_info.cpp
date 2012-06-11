#include <sasl/include/semantic/abi_info.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/host/utility.h>

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
using salviar::storage_usage_count;
using salviar::lang_pixel_shader;
using salviar::lang_vertex_shader;
using salviar::PACKAGE_ELEMENT_COUNT;
using salviar::SIMD_ELEMENT_COUNT;

using salviar::sv_layout;

using eflib::ceil_to_pow2;

using boost::addressof;
using boost::shared_ptr;

using std::vector;

BEGIN_NS_SASL_SEMANTIC();

using namespace sasl::utility::ops;

abi_info::abi_info()
	: mod(NULL), entry_point(NULL), lang(salviar::lang_none)
{
	memset( counts, 0, sizeof(counts) );
	memset( offsets, 0, sizeof(offsets) );
}

void abi_info::module( shared_ptr<module_semantic> const& v ){
	mod = v.get();
}

bool abi_info::is_module( shared_ptr<module_semantic> const& v ) const{
	return mod == v.get();
}

void abi_info::entry( shared_ptr<symbol> const& v ){
	entry_point = v.get();
	entry_point_name = entry_point->mangled_name();
}

bool abi_info::is_entry( shared_ptr<symbol> const& v ) const{
	return entry_point == v.get();
}

std::string abi_info::entry_name() const{
	return entry_point_name;
}

bool abi_info::add_input_semantic( salviar::semantic_value const& sem, builtin_types btc, bool is_stream )
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

bool abi_info::add_output_semantic( salviar::semantic_value const& sem, builtin_types btc, bool is_stream  ){
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

void abi_info::add_global_var(symbol* v, tynode_ptr tyn)
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

	si->internal_type = tyn->si_ptr<type_info_si>()->entry_id();
	si->usage = su_buffer_in;

	name_storages.insert( make_pair(v->unmangled_name(), si) );
}

sv_layout* abi_info::input_sv_layout( salviar::semantic_value const& sem ) const {
	sem_storages_t::const_iterator it = semin_storages.find( sem );
	if ( it == semin_storages.end() ){
		return NULL;
	}
	return const_cast<sv_layout*>( addressof( it->second ) );
}

sv_layout* abi_info::alloc_input_storage( salviar::semantic_value const& sem ){
	return addressof( semin_storages[sem] );
}

sv_layout* abi_info::input_sv_layout( symbol* v ) const {
	sym_storages_t::const_iterator it = symin_storages.find(v);
	if ( it == symin_storages.end() ){
		return NULL;
	}
	return const_cast<sv_layout*>( addressof( it->second ) );
}

sv_layout* abi_info::input_sv_layout( std::string const& name ) const
{
	name_storages_t::const_iterator it = name_storages.find( name );
	if( it == name_storages.end() ){
		return NULL;
	}
	return it->second;
}
sv_layout* abi_info::alloc_input_storage( symbol* v ){
	return addressof( symin_storages[v] );
}

sv_layout* abi_info::output_sv_layout( salviar::semantic_value const& sem ) const{
	sem_storages_t::const_iterator it = semout_storages.find( sem );
	if ( it == semout_storages.end() ){
		return NULL;
	}
	return const_cast<sv_layout*>( addressof( it->second ) );
}

size_t abi_info::total_size( sv_usage st ) const{
	return offsets[st];
}

sv_layout* abi_info::alloc_output_storage( salviar::semantic_value const& sem ){
	return addressof( semout_storages[sem] );
}

// Update ABI Information
void abi_info::compute_layout(){
	if ( !mod || !entry_point ) return;

	if ( lang == salviar::lang_general ){
		return;
	}

	switch( lang ){
	case lang_vertex_shader:
		compute_input_semantics_layout();
		compute_output_buffer_layout();
		compute_output_stream_layout();
		break;
	case lang_pixel_shader:
		compute_input_package_layout();
		compute_output_package_layout();
		break;
	}
	
	compute_input_constant_layout();
}

std::vector<sv_layout*> abi_info::layouts( sv_usage st ) const{
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

void abi_info::compute_input_semantics_layout(){
	for ( size_t index = 0; index < sems_in.size(); ++index ){

		sv_layout* svl = input_sv_layout( sems_in[index] );
		assert( svl );

		svl->physical_index = counts[svl->usage];
		svl->logical_index  = counts[svl->usage];
		svl->offset			= offsets[svl->usage];
		svl->element_count = 1;
		compute_element_size(svl, false);

		counts[svl->usage]++;
		offsets[svl->usage] += svl->total_size();

	}
}

void abi_info::compute_output_buffer_layout(){
	for ( size_t index = 0; index < sems_out.size(); ++index ){
		sv_layout* svl = output_sv_layout( sems_out[index] );
		assert(svl);

		svl->usage = su_buffer_out;
		svl->physical_index =  counts[svl->usage];
		svl->logical_index  =  counts[svl->usage];
		svl->offset = offsets[svl->usage];
		svl->element_count = 1;
		compute_element_size(svl, false);
		
		counts[svl->usage]++;
		offsets[svl->usage] += svl->total_size();
	}
}

void abi_info::compute_output_stream_layout()
{
	// Do nothing.
}

void abi_info::compute_input_constant_layout(){
	for ( size_t index = 0; index < syms_in.size(); ++index ){
		sv_layout* svl = addressof( symin_storages[ syms_in[index] ] );
		svl->usage = su_buffer_in;
		svl->physical_index =  counts[svl->usage];
		svl->logical_index  =  counts[svl->usage];
		svl->offset = offsets[su_buffer_in];

		svl->element_count = 1;
		compute_element_size(svl, false);

		counts[su_buffer_in]++;
		offsets[su_buffer_in] += svl->element_size;
	}
}

void abi_info::compute_input_package_layout()
{
	for ( size_t index = 0; index < sems_in.size(); ++index ){
		compute_package_layout( input_sv_layout( sems_in[index] ) );
	}
}

void abi_info::compute_output_package_layout()
{
	for ( size_t index = 0; index < sems_out.size(); ++index ){
		compute_package_layout( output_sv_layout( sems_out[index] ) );
	}
}

void abi_info::update_size( size_t sz, salviar::sv_usage usage )
{
	offsets[usage] = sz;
}

void abi_info::compute_package_layout( sv_layout* svl )
{
	assert( svl );

	svl->physical_index = counts[svl->usage];
	svl->logical_index  = counts[svl->usage];
	svl->offset = offsets[svl->usage];

	builtin_types elem_bt = to_builtin_types(svl->value_type);

	if( svl->usage == su_buffer_in || svl->usage == su_buffer_out ){
		svl->element_count = 1;
		compute_element_size(svl, true);
	} else {
		if( is_vector(elem_bt) || is_scalar( elem_bt ) ){
			compute_element_size(svl, true);
			int pow2_elem_size = ceil_to_pow2(svl->element_size);
			svl->element_count = PACKAGE_ELEMENT_COUNT;
			svl->element_padding = pow2_elem_size - svl->element_size;
		} else if ( is_matrix( elem_bt ) ){
			int row_size = static_cast<int>( storage_size( row_vector_of(elem_bt) ) );
			int pow2_row_size = ceil_to_pow2( row_size );
			int mat_size = pow2_row_size * static_cast<int>( vector_count(elem_bt) );
			int pow2_mat_size = ceil_to_pow2( mat_size );
			svl->element_size = mat_size;
			svl->element_padding = pow2_mat_size - mat_size;
			svl->element_count = PACKAGE_ELEMENT_COUNT;
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}

	counts[svl->usage]++;
	offsets[svl->usage] += svl->total_size();
}

int abi_info::compute_element_size(sv_layout* svl, bool package) const
{
	int elem_sz = 0;
	if( (svl->usage != su_buffer_in && !package) || svl->agg_type == salviar::aggt_array )
	{
		elem_sz = static_cast<int>( sizeof(void*) );
	}
	else
	{
		elem_sz = static_cast<int>( storage_size( to_builtin_types(svl->value_type) ) );
	}
	svl->element_size = elem_sz;

	return elem_sz;
}

END_NS_SASL_SEMANTIC();