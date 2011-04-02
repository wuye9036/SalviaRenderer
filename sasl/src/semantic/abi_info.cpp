#include <sasl/include/semantic/abi_info.h>

#include <sasl/include/semantic/semantic_infos.h>
#include <eflib/include/diagnostics/assert.h>

using boost::addressof;
using boost::shared_ptr;
using std::vector;

BEGIN_NS_SASL_SEMANTIC();

storage_info::storage_info()
	: index(-1), offset(0), size(0), storage(storage_none), sv_type( builtin_type_code::none )
{}

abi_info::abi_info()
	: mod(NULL), entry_point(NULL), lang(softart::lang_none),
	sems_in_size(0)
{}

void abi_info::module( shared_ptr<module_si> const& v ){
	mod = v.get();
}

bool abi_info::is_module( shared_ptr<module_si> const& v ) const{
	return mod == v.get();
}

void abi_info::entry( shared_ptr<symbol> const& v ){
	entry_point = v.get();
}

bool abi_info::is_entry( shared_ptr<symbol> const& v ) const{
	return entry_point == v.get();
}

bool abi_info::add_input_semantic( softart::semantic sem, builtin_type_code btc )
{
	vector<softart::semantic>::iterator it = std::lower_bound( sems_in.begin(), sems_in.end(), sem );
	if( it != sems_in.end() ){
		if( *it == sem ){
			storage_info* si = alloc_input_storage( sem );
			if( si->sv_type == btc || si->sv_type == builtin_type_code::none ){
				si->sv_type = btc;
				return true;
			}
			return false;
		}
	}

	sems_in.insert( it, sem );
	return true;
}

bool abi_info::add_output_semantic( softart::semantic sem, builtin_type_code btc ){
	vector<softart::semantic>::iterator it = std::lower_bound( sems_out.begin(), sems_out.end(), sem );
	if( it != sems_out.end() ){
		if( *it == sem ){
			storage_info* si = alloc_output_storage( sem );
			if( si->sv_type == btc || si->sv_type == builtin_type_code::none ){
				si->sv_type = btc;
				return true;
			}
			return false;
		}
	}

	sems_out.insert( it, sem );
	return true;
}

void abi_info::add_global_var( boost::shared_ptr<symbol> const& v, builtin_type_code btc )
{
	syms_in.push_back( v.get() );
	storage_info* si = alloc_input_storage( v );
	si->sv_type = btc;
}

storage_info* abi_info::input_storage( softart::semantic sem ){
	sem_storages_t::iterator it = semin_storages.find( sem );
	if ( it == semin_storages.end() ){
		return NULL;
	}
	return addressof( it->second );
}

storage_info* abi_info::alloc_input_storage( softart::semantic sem ){
	return addressof( semin_storages[sem] );
}

storage_info* abi_info::input_storage( boost::shared_ptr<symbol> const& v ){
	sym_storages_t::iterator it = symin_storages.find( v.get() );
	if ( it == symin_storages.end() ){
		return NULL;
	}
	return addressof( it->second );
}

storage_info* abi_info::alloc_input_storage( boost::shared_ptr<symbol> const& v ){
	return addressof( symin_storages[v.get()] );
}

storage_info* abi_info::output_storage( softart::semantic sem ){
	sem_storages_t::iterator it = semout_storages.find( sem );
	if ( it == semout_storages.end() ){
		return NULL;
	}
	return addressof( it->second );
}

storage_info* abi_info::alloc_output_storage( softart::semantic sem ){
	return addressof( semout_storages[sem] );
}

// Update ABI Information
void abi_info::compute_layout(){
	if ( !mod || !entry_point ) return;

	compute_input_semantics_layout();
	compute_input_stream_layout();
	compute_output_semantics_layout();
	compute_output_stream_layout();
	compute_input_constant_layout();
}

void abi_info::compute_input_semantics_layout(){
	int offset = 0;
	for ( size_t index = 0; index < sems_in.size(); ++index ){
		storage_info* pstorage = alloc_input_storage( sems_in[index] );
		pstorage->storage = buffer_in;
		pstorage->index = static_cast<int>( index );
		pstorage->offset = offset;
		int size = static_cast<int>( sasl_ehelper::storage_size( pstorage->sv_type ) );
		pstorage->size = size;
		offset += size;
	}
	sems_in_size = offset;
}

void abi_info::compute_input_stream_layout(){
	int const ptr_size = static_cast<int>( sizeof( void* ) );
	for ( size_t index = 0; index < sems_in.size(); ++index ){
		storage_info* pstorage = alloc_input_storage( sems_in[index] );
		pstorage->storage = stream_in;
		pstorage->index =  static_cast<int>( index );
		pstorage->offset = static_cast<int>( index ) * ptr_size;
		pstorage->size = ptr_size;
	}
}

void abi_info::compute_output_semantics_layout(){
	int offset = 0;
	for ( size_t index = 0; index < sems_in.size(); ++index ){
		storage_info* pstorage = alloc_output_storage( sems_in[index] );
		pstorage->storage = buffer_out;
		pstorage->index =  static_cast<int>( index );
		pstorage->offset = offset;
		int size = static_cast<int>( sasl_ehelper::storage_size( pstorage->sv_type ) );
		pstorage->size = size;
		offset += size;
	}
}

void abi_info::compute_output_stream_layout()
{
	// Do nothing.
}

void abi_info::compute_input_constant_layout(){
	int offset = sems_in_size;
	int index_base = static_cast<int>( sems_in.size() );

	for ( size_t index = 0; index < syms_in.size(); ++index ){
		storage_info* pstorage = addressof( symin_storages[ syms_in[index] ] );
		pstorage->storage = buffer_in;
		pstorage->index = static_cast<int>( index + index_base );
		pstorage->offset = offset;
		int size = static_cast<int>( sasl_ehelper::storage_size( pstorage->sv_type ) );
		pstorage->size = size;
		offset += size;
	}
	sems_in_size = offset;
}


END_NS_SASL_SEMANTIC();