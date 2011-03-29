#include <sasl/include/semantic/abi_analyser.h>

#include <sasl/enums/builtin_type_code.h>
#include <sasl/enums/enums_helper.h>

#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/utility/addressof.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

using namespace sasl::syntax_tree;

using boost::addressof;
using boost::make_shared;
using boost::shared_ptr;

using std::lower_bound;
using std::string;
using std::vector;


BEGIN_NS_SASL_SEMANTIC();

bool verify_semantic_type( builtin_type_code btc, softart::semantic sem ){
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

storage_types vsinput_semantic_storage( softart::semantic sem ){
	switch( semantic_base(sem) ){
	case softart::SV_Position:
		return stream_in;
	case softart::SV_Texcoord:
		return stream_in;
	}
	return storage_none;
}

storage_types vsoutput_semantic_storage( softart::semantic sem ){
	switch( semantic_base(sem) ){
	case softart::SV_Position:
		return buffer_out;
	case softart::SV_Texcoord:
		return buffer_out;
	}
	return storage_none;
}

storage_types semantic_storage( softart::languages lang, bool is_output, softart::semantic sem ){
	switch ( lang ){
	case softart::lang_vertex_sl:
		if( is_output ){
			return vsoutput_semantic_storage(sem);
		} else {
			return vsinput_semantic_storage(sem);
		}
	}
	return storage_none;
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

bool abi_analyser::entry( shared_ptr<module_si> const& mod, string const& name, softart::languages lang ){
	vector< shared_ptr<symbol> > const& overloads = mod->root()->find_overloads( name );
	if ( overloads.size() != 1 ){
		return false;
	}

	return entry( mod, overloads[0], lang );
}

bool abi_analyser::entry( shared_ptr<module_si> const& mod, shared_ptr<symbol> const& fnsym, softart::languages lang ){
	assert( lang < softart::lang_count );

	mods[lang] = mod;
	entries[lang] = fnsym;
	abiis[lang].reset();

	return update_abiis();
}

bool abi_analyser::auto_entry( shared_ptr<module_si> const& mod, softart::languages lang ){
	shared_ptr<symbol> candidate;
	shared_ptr<abi_info> candidate_abii;
	BOOST_FOREACH( shared_ptr<symbol> const& fnsym, mod->functions() ){
		if( entry(mod, fnsym, lang) ){
			if( candidate ){
				// TODO More than one mactched. conflict error.
				reset(lang);
				return false;
			}
			candidate = fnsym;
			candidate_abii = abiis[lang];
		}
	}

	if( candidate ){
		mods[lang] = mod;
		entries[lang] = candidate;
		abiis[lang] = candidate_abii;
		return true;
	}

	return false;
}

boost::shared_ptr<symbol> const& abi_analyser::entry( softart::languages lang ) const{
	return entries[lang];
}

bool abi_analyser::update_abiis(){
	return update( softart::lang_vertex_sl )
		&& update( softart::lang_pixel_sl )
		&& update( softart::lang_blend_sl )
		;
}

bool abi_analyser::verify_abiis(){
	return verify_vs_ps() && verify_ps_bs();
}

bool abi_analyser::update( softart::languages lang ){
	if ( abiis[lang] ){
		return true;
	}

	if( ! (mods[lang] && entries[lang]) ){
		return false;
	}

	abiis[lang] = make_shared<abi_info>();

	shared_ptr<function_type> entry_fn = entries[lang]->node()->typed_handle<function_type>();
	assert( entry_fn );

	if( !add_semantic( entry_fn, false, false, softart::lang_vertex_sl, true ) ){
		reset(lang);
		return false;
	}

	BOOST_FOREACH( shared_ptr<parameter> const& param, entry_fn->params )
	{
		if( !add_semantic( param, false, false, softart::lang_vertex_sl, false ) ){
			reset(lang);
			return false;
		}
	}

	// TODO processes global variables.
	EFLIB_ASSERT_UNIMPLEMENTED();

	return false;
}

abi_info const* abi_analyser::abii( softart::languages lang ) const{
	return abiis[lang].get();
}

abi_info* abi_analyser::abii( softart::languages lang ){
	return abiis[lang].get();
}

bool abi_analyser::add_semantic(
	shared_ptr<node> const& v,
	bool is_member, bool enable_nested,
	softart::languages lang, bool is_output)
{
	abi_info* ai = abii(lang);
	assert( ai );
	storage_si* pssi = dynamic_cast<storage_si*>( v->semantic_info().get() );
	assert(pssi); // TODO Here are semantic analysis error.
	type_specifier* ptspec = pssi->type_info().get();
	assert(ptspec); // TODO Here are semantic analysis error.

	softart::semantic node_sem = pssi->get_semantic();

	if( ptspec->is_builtin() ){
		builtin_type_code btc = ptspec->value_typecode;
		if ( verify_semantic_type( btc, node_sem ) ) {
			storage_types sem_s = semantic_storage( lang, is_output, node_sem );
			switch( sem_s ){

			case stream_in:
			case buffer_in:
				return ai->add_input_semantic( node_sem, btc );

			case buffer_out:
			case stream_out:
				return ai->add_output_semantic( node_sem, btc );
			}

			return false;
		}
	} else if( ptspec->node_class() == syntax_node_types::struct_type ){
		if( is_member && !enable_nested ){
			return false;
		}

		// TODO do not support nested aggregated variable. 
		struct_type* pstructspec = dynamic_cast<struct_type*>( ptspec );
		assert( pstructspec );
		BOOST_FOREACH( shared_ptr<declaration> const& decl, pstructspec->decls )
		{
			if ( decl->node_class() == syntax_node_types::variable_declaration ){
				shared_ptr<variable_declaration> vardecl = decl->typed_handle<variable_declaration>();
				BOOST_FOREACH( shared_ptr<declarator> const& dclr, vardecl->declarators ){
					if ( !add_semantic( dclr, true, enable_nested, lang, is_output ) ){
						return false;
					}
				}
			}
		}

		return true;
	}

	return false;
}

bool abi_analyser::verify_vs_ps(){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return false;
}

bool abi_analyser::verify_ps_bs(){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return false;
}

END_NS_SASL_SEMANTIC();