#include <sasl/include/semantic/abi_analyser.h>

#include <sasl/include/semantic/abi_info.h>

#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/utility/addressof.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

using namespace sasl::syntax_tree;
using namespace sasl::utility;

using salviar::sv_usage;
using salviar::su_none;
using salviar::su_stream_in;
using salviar::su_stream_out;
using salviar::su_buffer_in;
using salviar::su_buffer_out;
using salviar::storage_usage_count;

using boost::addressof;
using boost::make_shared;
using boost::shared_ptr;

using std::lower_bound;
using std::string;
using std::vector;

BEGIN_NS_SASL_SEMANTIC();

bool verify_semantic_type( builtin_types btc, salviar::semantic_value const& sem ){
	switch( sem.get_system_value() ){

	case salviar::sv_none:
		return false;

	case salviar::sv_position:
	case salviar::sv_texcoord:
	case salviar::sv_normal:
	case salviar::sv_target:
		return 
			( is_scalar(btc) || is_vector(btc) || is_matrix(btc) )
			&& ( scalar_of(btc) == builtin_types::_float || scalar_of(btc) == builtin_types::_sint32 );
	case salviar::sv_blend_indices:
		return ( is_scalar(btc) || is_vector(btc) ) && is_integer( scalar_of(btc) );
	case salviar::sv_blend_weights:
		return ( is_scalar(btc) || is_vector(btc) ) && ( scalar_of(btc) == builtin_types::_float );
	case salviar::sv_depth:
		return ( btc == builtin_types::_float );
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return false;
}

sv_usage vsinput_semantic_usage( salviar::semantic_value const& sem ){
	switch( sem.get_system_value() ){
	case salviar::sv_position:
	case salviar::sv_texcoord:
	case salviar::sv_normal:
	case salviar::sv_blend_indices:
	case salviar::sv_blend_weights:
		return su_stream_in;
	}
	EFLIB_ASSERT_UNIMPLEMENTED();
	return su_none;
}

sv_usage vsoutput_semantic_usage( salviar::semantic_value const& sem ){
	switch( sem.get_system_value() ){
	case salviar::sv_position:
		return su_buffer_out;
	case salviar::sv_texcoord:
		return su_buffer_out;
	}
	EFLIB_ASSERT_UNIMPLEMENTED();
	return su_none;
}

sv_usage psinput_semantic_usage( salviar::semantic_value const& sem ){
	switch( sem.get_system_value() ){
	case salviar::sv_texcoord:
		return su_stream_in;
	}
	EFLIB_ASSERT_UNIMPLEMENTED();
	return su_none;
}

sv_usage psoutput_semantic_usage( salviar::semantic_value const& sem ){
	switch( sem.get_system_value() ){
	case salviar::sv_target:
		return su_stream_out;
	}
	EFLIB_ASSERT_UNIMPLEMENTED();
	return su_none;
}

sv_usage semantic_usage( salviar::languages lang, bool is_output, salviar::semantic_value const& sem ){
	switch ( lang ){
	case salviar::lang_vertex_shader:
		if( is_output ){
			return vsoutput_semantic_usage(sem);
		} else {
			return vsinput_semantic_usage(sem);
		}
	case salviar::lang_pixel_shader:
		if( is_output ){
			return psoutput_semantic_usage(sem);
		} else {
			return psinput_semantic_usage(sem);
		}
	}

	return su_none;
}

void abi_analyser::reset( salviar::languages lang ){
	assert( lang < salviar::lang_count );
	
	mods[lang].reset();
	entries[lang] = NULL;
	abiis[lang].reset();
}

void abi_analyser::reset_all(){
	for( int i = 0; i < salviar::lang_count; ++i ){
		reset( static_cast<salviar::languages>(i) );
	}
}

bool abi_analyser::entry( shared_ptr<module_semantic> const& mod, string const& name, salviar::languages lang ){
	vector< shared_ptr<symbol> > const& overloads = mod->root_symbol()->find_overloads( name );
	if ( overloads.size() != 1 ){
		return false;
	}

	return entry( mod, overloads[0].get(), lang );
}

bool abi_analyser::entry( module_semantic_ptr const& mod, symbol* fnsym, salviar::languages lang ){
	assert( lang < salviar::lang_count );

	mods[lang] = mod;
	entries[lang] = fnsym;
	abiis[lang].reset();
	return update(lang);
}

bool abi_analyser::auto_entry( shared_ptr<module_semantic> const& mod, salviar::languages lang ){
	symbol*			candidate = NULL;
	abi_info_ptr	candidate_abii;

	BOOST_FOREACH( symbol* fnsym, mod->functions() ){
		if( entry(mod, fnsym, lang) ){
			if( candidate ){
				// TODO More than one matched. conflict error.
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

symbol* abi_analyser::entry( salviar::languages lang ) const{
	return entries[lang];
}

bool abi_analyser::update_abiis(){
	return update( salviar::lang_vertex_shader )
		&& update( salviar::lang_pixel_shader )
		&& update( salviar::lang_blending_shader )
		;
}

bool abi_analyser::verify_abiis(){
	return verify_vs_ps() && verify_ps_bs();
}

bool abi_analyser::update( salviar::languages lang ){
	if ( abiis[lang] ){
		return true;
	}

	if( ! (mods[lang] && entries[lang]) ){
		return false;
	}

	// Initialize language ABI information.
	abiis[lang] = make_shared<abi_info>();
	abiis[lang]->lang = lang;
	abiis[lang]->mod = mods[lang].get();
	abiis[lang]->entry_point = entries[lang];
	
	if( entries[lang] ){
		abiis[lang]->entry_point_name = entries[lang]->mangled_name();
	}

	if( lang == salviar::lang_vertex_shader
		|| lang == salviar::lang_pixel_shader
		|| lang == salviar::lang_blending_shader
		)
	{
		// Process entry function.
		shared_ptr<function_type> entry_fn = entries[lang]->node()->as_handle<function_type>();
		assert( entry_fn );

		if( !add_semantic( entry_fn, false, false, lang, true ) ){
			assert( false );
			reset(lang);
			return false;
		}

		BOOST_FOREACH( shared_ptr<parameter> const& param, entry_fn->params )
		{
			if( !add_semantic( param, false, false, lang, false ) ){
				reset(lang);
				return false;
			}
		}

		// Process global variables.
		BOOST_FOREACH( symbol* gvar_sym, mods[lang]->global_vars() ){
			shared_ptr<declarator> gvar = gvar_sym->node()->as_handle<declarator>();
			assert(gvar);

			// is_member is set to true for preventing aggregated variable.
			// And global variable only be treated as input.
			if( !add_semantic( gvar, true, false, lang, false ) ){
				// If it is not attached to an valid semantic, it should be uniform variable.

				// Check the data type of global. Now global variables only support built-in types.
				storage_si* psi = dynamic_cast<storage_si*>( gvar->semantic_info().get() );
				if( psi->type_info()->is_builtin() || psi->type_info()->is_array()  )
				{
					abiis[lang]->add_global_var(gvar_sym, psi->type_info() );
				}
				else
				{
					//TODO It an semantic error need to be reported.
					return false;
				}
			}
		}
	}
	
	// Compute ABI memory layout.
	abiis[lang]->compute_layout();

	return true;
}

abi_info const* abi_analyser::abii( salviar::languages lang ) const{
	return abiis[lang].get();
}

abi_info* abi_analyser::abii( salviar::languages lang ){
	return abiis[lang].get();
}

bool abi_analyser::add_semantic(
	shared_ptr<node> const& v,
	bool is_member, bool enable_nested,
	salviar::languages lang, bool is_output)
{
	abi_info* ai = abii(lang);
	assert( ai );
	storage_si* pssi = dynamic_cast<storage_si*>( v->semantic_info().get() );
	assert(pssi); // TODO Here are semantic analysis error.
	tynode* ptspec = pssi->type_info().get();
	assert(ptspec); // TODO Here are semantic analysis error.

	salviar::semantic_value const& node_sem = pssi->get_semantic();

	if( ptspec->is_builtin() ){
		builtin_types btc = ptspec->tycode;
		if ( verify_semantic_type( btc, node_sem ) ) {
			sv_usage sem_s = semantic_usage( lang, is_output, node_sem );
			switch( sem_s ){

			case su_stream_in:
				return ai->add_input_semantic( node_sem, btc, true );
			case su_buffer_in:
				return ai->add_input_semantic( node_sem, btc, false );
			case su_stream_out:
				return ai->add_output_semantic( node_sem, btc, true );
			case su_buffer_out:
				return ai->add_output_semantic( node_sem, btc, false );
			}

			assert( false );
			return false;
		}
	} else if( ptspec->node_class() == node_ids::struct_type ){
		if( is_member && !enable_nested ){
			return false;
		}

		// TODO do not support nested aggregated variable. 
		struct_type* pstructspec = dynamic_cast<struct_type*>( ptspec );
		assert( pstructspec );
		BOOST_FOREACH( shared_ptr<declaration> const& decl, pstructspec->decls )
		{
			if ( decl->node_class() == node_ids::variable_declaration ){
				shared_ptr<variable_declaration> vardecl = decl->as_handle<variable_declaration>();
				BOOST_FOREACH( shared_ptr<declarator> const& dclr, vardecl->declarators ){
					if ( !add_semantic( dclr, true, enable_nested, lang, is_output ) ){
						assert( false );
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

shared_ptr<abi_info> abi_analyser::shared_abii( salviar::languages lang ) const{
	return abiis[lang];
}

END_NS_SASL_SEMANTIC();