#include <sasl/include/semantic/abi_info.h>

#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using boost::addressof;
using boost::shared_ptr;
using std::vector;

BEGIN_NS_SASL_SEMANTIC();

storage_info::storage_info()
	: index(-1), offset(0), size(0), storage(storage_none), sv_type( builtin_types::none )
{
}

abi_info::abi_info()
	: mod(NULL), entry_point(NULL), lang(salviar::lang_none)
{
	memset( counts, 0, sizeof(counts) );
	memset( offsets, 0, sizeof(offsets) );
}

void abi_info::module( shared_ptr<module_si> const& v ){
	mod = v.get();
}

bool abi_info::is_module( shared_ptr<module_si> const& v ) const{
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
	vector<salviar::semantic_value const&>::iterator it = std::lower_bound( sems_in.begin(), sems_in.end(), sem );
	if( it != sems_in.end() ){
		if( *it == sem ){
			storage_info* si = input_storage( sem );
			assert(si);
			if( si->sv_type == btc || si->sv_type == builtin_types::none ){
				si->sv_type = btc;
				return true;
			}
			return false;
		}
	}

	storage_info* si = alloc_input_storage( sem );
	si->sv_type = btc;
	si->storage = is_stream ? sc_stream_in : sc_buffer_in;
	sems_in.insert( it, sem );
	return true;
}

bool abi_info::add_output_semantic( salviar::semantic_value const& sem, builtin_types btc ){
	vector<salviar::semantic_value const&>::iterator it = std::lower_bound( sems_out.begin(), sems_out.end(), sem );
	if( it != sems_out.end() ){
		if( *it == sem ){
			storage_info* si = alloc_output_storage( sem );
			if( si->sv_type != btc && si->sv_type == builtin_types::none ){
				si->sv_type = btc;
				return true;
			}
			return false;
		}
	}

	storage_info* si = alloc_output_storage( sem );
	si->sv_type = btc;
	si->storage = sc_buffer_out;
	sems_out.insert( it, sem );
	return true;
}

void abi_info::add_global_var( boost::shared_ptr<symbol> const& v, builtin_types btc )
{
	syms_in.push_back( v.get() );
	storage_info* si = alloc_input_storage( v );
	si->sv_type = btc;
	si->storage = sc_buffer_in;

	name_storages.insert( make_pair(v->unmangled_name(), si) );
}

storage_info* abi_info::input_storage( salviar::semantic_value const& sem ) const {
	sem_storages_t::const_iterator it = semin_storages.find( sem );
	if ( it == semin_storages.end() ){
		return NULL;
	}
	return const_cast<storage_info*>( addressof( it->second ) );
}

storage_info* abi_info::alloc_input_storage( salviar::semantic_value const& sem ){
	return addressof( semin_storages[sem] );
}

storage_info* abi_info::input_storage( boost::shared_ptr<symbol> const& v ) const {
	sym_storages_t::const_iterator it = symin_storages.find( v.get() );
	if ( it == symin_storages.end() ){
		return NULL;
	}
	return const_cast<storage_info*>( addressof( it->second ) );
}

storage_info* abi_info::input_storage( std::string const& name ) const
{
	name_storages_t::const_iterator it = name_storages.find( name );
	if( it == name_storages.end() ){
		return NULL;
	}
	return it->second;
}
storage_info* abi_info::alloc_input_storage( boost::shared_ptr<symbol> const& v ){
	return addressof( symin_storages[v.get()] );
}

storage_info* abi_info::output_storage( salviar::semantic_value const& sem ) const{
	sem_storages_t::const_iterator it = semout_storages.find( sem );
	if ( it == semout_storages.end() ){
		return NULL;
	}
	return const_cast<storage_info*>( addressof( it->second ) );
}

size_t abi_info::storage_size( storage_classifications st ) const{
	return offsets[st];
}

storage_info* abi_info::alloc_output_storage( salviar::semantic_value const& sem ){
	return addressof( semout_storages[sem] );
}

// Update ABI Information
void abi_info::compute_layout(){
	if ( !mod || !entry_point ) return;

	if ( lang == salviar::lang_general ){
		return;
	}

	compute_input_semantics_layout();
	compute_output_buffer_layout();
	compute_output_stream_layout();
	compute_input_constant_layout();
}

std::vector<storage_info*> abi_info::storage_infos( storage_classifications st ) const{
	std::vector<storage_info*> ret;

	// Process output
	if( st == sc_buffer_out ){
		BOOST_FOREACH( salviar::semantic_value const& sem, sems_out ){
			ret.push_back( output_storage(sem) );
		}
		return ret;
	}

	// Process input
	for ( size_t index = 0; index < sems_in.size(); ++index ){
		storage_info* pstorage = input_storage( sems_in[index] );
		if ( pstorage->storage == st ){
			ret.push_back( pstorage );
		}
	}

	if ( st == sc_buffer_in ){
		BOOST_FOREACH( symbol* sym, syms_in ){
			ret.push_back( const_cast<storage_info*>( addressof( symin_storages.find(sym)->second ) ) );
		}
	}

	return ret;
}

void abi_info::compute_input_semantics_layout(){
	for ( size_t index = 0; index < sems_in.size(); ++index ){

		storage_info* pstorage = input_storage( sems_in[index] );
		assert( pstorage );

		pstorage->index =  counts[pstorage->storage];
		pstorage->offset = offsets[pstorage->storage];
		pstorage->size = 
			pstorage->storage == sc_buffer_in ?
			static_cast<int>( storage_size( pstorage->sv_type ) )
			: static_cast<int> ( sizeof(void*) )
			;

		counts[pstorage->storage]++;
		offsets[pstorage->storage] += pstorage->size;

	}
}

void abi_info::compute_output_buffer_layout(){
	for ( size_t index = 0; index < sems_out.size(); ++index ){
		storage_info* pstorage = output_storage( sems_out[index] );
		assert(pstorage);

		pstorage->storage = sc_buffer_out;
		pstorage->index =  counts[pstorage->storage];
		pstorage->offset = offsets[pstorage->storage];
		pstorage->size = static_cast<int>( storage_size( pstorage->sv_type ) );
		
		counts[pstorage->storage]++;
		offsets[pstorage->storage] += pstorage->size;
	}
}

void abi_info::compute_output_stream_layout()
{
	// Do nothing.
}

void abi_info::compute_input_constant_layout(){
	for ( size_t index = 0; index < syms_in.size(); ++index ){
		storage_info* pstorage = addressof( symin_storages[ syms_in[index] ] );
		pstorage->storage = sc_buffer_in;
		pstorage->index = counts[sc_buffer_in];
		pstorage->offset = offsets[sc_buffer_in];

		int size = static_cast<int>( storage_size( pstorage->sv_type ) );
		pstorage->size = size;

		counts[sc_buffer_in]++;
		offsets[sc_buffer_in] += size;
	}
}

END_NS_SASL_SEMANTIC();