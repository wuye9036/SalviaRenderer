#include <sasl/include/semantic/abi_analyser.h>

#include <sasl/enums/builtin_type_code.h>
#include <sasl/enums/enums_helper.h>

#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/utility/addressof.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

using namespace sasl::syntax_tree;

using boost::addressof;
using boost::make_shared;
using boost::shared_ptr;

using std::lower_bound;
using std::vector;


BEGIN_NS_SASL_SEMANTIC();

bool verify_semantic( builtin_type_code btc, softart::semantic sem ){
	switch( semantic_base(sem) ){

	case softart::SV_None:
		return false;

	case softart::SV_Position:
	case softart::SV_Texcoord:
		return 
			( sasl_ehelper::is_scalar(btc) || sasl_ehelper::is_vector(btc) )
			&& sasl_ehelper::scalar_of(btc) == builtin_type_code::_float;
	}

	return false;
}

storage_info::storage_info()
	: index(-1), offset(0), size(0), storage(storage_none), sv_type( builtin_type_code::none )
{}

abi_info::abi_info() : mod(NULL), entry_point(NULL), lang(softart::lang_none)
{}

void abi_info::module( shared_ptr<module_si> const& v ){
	mod = v.get();
}

bool abi_info::is_module( boost::shared_ptr<module_si> const& v ) const{
	return mod == v.get();
}

void abi_info::entry( boost::shared_ptr<symbol> const& v ){
	entry_point = v.get();
}

bool abi_info::is_entry( boost::shared_ptr<symbol> const& v ) const{
	return entry_point == v.get();
}

bool abi_info::add_input_semantic( softart::semantic sem ){
	vector<softart::semantic>::iterator it = std::lower_bound( sems_in.begin(), sems_in.end(), sem );
	if( it != sems_in.end() ){
		if( *it == sem ){
			return false;
		}
	}

	sems_in.insert( it, sem );
	return true;
}

bool abi_info::add_output_semantic( softart::semantic sem ){
	vector<softart::semantic>::iterator it = std::lower_bound( sems_in.begin(), sems_in.end(), sem );
	if( it != sems_in.end() ){
		if( *it == sem ){
			return false;
		}
	}

	sems_in.insert( it, sem );
	return true;
}

void abi_info::add_global_var( shared_ptr<symbol> const& v ){
	syms_in.push_back( v.get() );
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
void abi_info::update_abii(){
	if ( !mod || !entry_point ) return;

	update_input_semantics_abii();
	update_input_stream_abii();
	update_output_semantics_abii();
	update_output_stream_abii();
}

void abi_info::update_input_semantics_abii(){
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
}

void abi_info::update_input_stream_abii(){
	int const ptr_size = static_cast<int>( sizeof( void* ) );
	for ( size_t index = 0; index < sems_in.size(); ++index ){
		storage_info* pstorage = alloc_input_storage( sems_in[index] );
		pstorage->storage = stream_in;
		pstorage->index =  static_cast<int>( index );
		pstorage->offset = static_cast<int>( index ) * ptr_size;
		pstorage->size = ptr_size;
	}
}

void abi_info::update_output_semantics_abii(){
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

void abi_info::update_output_stream_abii()
{
	// Do nothing.
}


void abi_analyser::reset( softart::languages lang ){
	assert( lang < softart::lang_count );
	
	mods[lang].reset();
	entries[lang].reset();
	abiis[lang].reset();
}


void abi_analyser::reset_all(){
	for( int i = 0; i < softart::lang_count; ++i ){
		reset( static_cast<softart::languages>(i) );
	}
}

bool abi_analyser::entry( boost::shared_ptr<module_si>& mod, std::string const& name, softart::languages lang ){
	assert( lang < softart::lang_count );

	mods[lang] = mod;
	vector< shared_ptr<symbol> > const& overloads = mod->root()->find_overloads( name );
	if ( overloads.size() != 1 ){
		return false;
	}

	entries[lang] = overloads[0];
	abiis[lang].reset();

	return update_abiis();
}

boost::shared_ptr<symbol> const& abi_analyser::entry( softart::languages lang ) const{
	return entries[lang];
}

bool abi_analyser::update_abiis(){
	return update_vs() && update_ps() && update_bs();
}

bool abi_analyser::verify_abiis(){
	return verify_vs_ps() && verify_ps_bs();
}

bool abi_analyser::update_vs(){
	softart::languages const lang = softart::lang_vertex_sl;

	if ( abiis[lang] ){
		return true;
	}

	if( ! (mods[lang] && entries[lang]) ){
		return false;
	}

	abiis[lang] = make_shared<abi_info>();

	shared_ptr<function_type> entry_fn = entries[lang]->node()->typed_handle<function_type>();

	shared_ptr<type_specifier> ret_type = entry_fn->retval_type;
	shared_ptr<storage_si> ssi = extract_semantic_info<storage_si>(ret_type);
	shared_ptr<type_specifier> real_type = ssi->type_info();

	if( real_type->is_builtin() ){
		// Verify built-in semantics.
		verify_semantic( real_type->value_typecode, ssi->get_semantic() );
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return false;
}

END_NS_SASL_SEMANTIC();