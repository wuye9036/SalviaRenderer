#include <sasl/include/semantic/reflector.h>

#include <sasl/include/semantic/reflection_impl.h>
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
using salviar::sv_usage_count;

using eflib::fixed_string;

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

reflection_impl_ptr reflector::reflect(
	module_semantic_ptr const& sem, fixed_string const& entry_name )
{
	vector<symbol*> overloads = sem->root_symbol()->find_overloads(entry_name);
	if ( overloads.size() != 1 )
	{
		return reflection_impl_ptr();
	}
	sem_	= sem.get();
	entry_	= overloads[0];
	return do_reflect();
}

reflection_impl_ptr reflector::do_reflect()
{
	if( ! (sem_ && entry_) )
	{
		return reflection_impl_ptr();
	}

	salviar::languages lang = sem_->get_language();

	// Initialize language ABI information.
	reflection_impl_ptr ret = make_shared<reflection_impl>();
	ret->lang				= lang;
	ret->module_sem_			= sem_;
	ret->entry_point_		= entry_;
	ret->entry_point_name_	= entry_->mangled_name();
	reflection_ = ret.get();
	
	if( lang == salviar::lang_vertex_shader
		|| lang == salviar::lang_pixel_shader
		|| lang == salviar::lang_blending_shader
		)
	{
		// Process entry function.
		shared_ptr<function_def> entry_fn = entry_->associated_node()->as_handle<function_def>();
		assert( entry_fn );

		if( !add_semantic(entry_fn, false, false, lang, true) )
		{
			assert( false );
			ret.reset();
			return ret;
		}

		BOOST_FOREACH( shared_ptr<parameter> const& param, entry_fn->params )
		{
			if( !add_semantic(param, false, false, lang, false) )
			{
				ret.reset();
				return ret;
			}
		}

		// Process global variables.
		BOOST_FOREACH( symbol* gvar_sym, sem_->global_vars() ){
			shared_ptr<declarator> gvar = gvar_sym->associated_node()->as_handle<declarator>();
			assert(gvar);

			// is_member is set to true for preventing aggregated variable.
			// And global variable only be treated as input.
			if( !add_semantic(gvar, true, false, lang, false) ){
				// If it is not attached to an valid semantic, it should be uniform variable.

				// Check the data type of global. Now global variables only support built-in types.
				node_semantic* psi = sem_->get_semantic( gvar.get() );
				if( psi->ty_proto()->is_builtin() || psi->ty_proto()->is_array()  )
				{
					ret->add_global_var(gvar_sym, psi->ty_proto()->as_handle<tynode>() );
				}
				else
				{
					//TODO: It an semantic error need to be reported.
					ret.reset();
					return ret;
				}
			}
		}
	}

	return ret;
}

reflection_impl_ptr reflector::reflect(module_semantic_ptr const& sem)
{
	symbol*				candidate = NULL;
	reflection_impl_ptr	candidate_reflection;

	BOOST_FOREACH( symbol* fn_sym, sem_->functions() )
	{
		entry_	= fn_sym;
		sem_	= sem.get();

		candidate_reflection = do_reflect();

		if(candidate_reflection)
		{
			if(candidate)
			{
				// TODO: More than one matched. conflict error.
				return reflection_impl_ptr();
			}
			candidate = fn_sym;
		}
	}

	return candidate_reflection;
}

bool reflector::add_semantic(
	shared_ptr<node> const& v,
	bool is_member, bool enable_nested,
	salviar::languages lang, bool is_output)
{
	assert(reflection_);
	node_semantic* pssi = sem_->get_semantic( v.get() );
	assert(pssi); // TODO: Here are semantic analysis error.
	tynode* ptspec = pssi->ty_proto();
	assert(ptspec); // TODO: Here are semantic analysis error.

	salviar::semantic_value const& node_sem = pssi->semantic_value_ref();

	if( ptspec->is_builtin() )
	{
		builtin_types btc = ptspec->tycode;
		if ( verify_semantic_type( btc, node_sem ) ) {
			sv_usage sem_s = semantic_usage( lang, is_output, node_sem );
			switch( sem_s ){

			case su_stream_in:
				return reflection_->add_input_semantic( node_sem, btc, true );
			case su_buffer_in:
				return reflection_->add_input_semantic( node_sem, btc, false );
			case su_stream_out:
				return reflection_->add_output_semantic( node_sem, btc, true );
			case su_buffer_out:
				return reflection_->add_output_semantic( node_sem, btc, false );
			}

			assert( false );
			return false;
		}
	}
	else if( ptspec->node_class() == node_ids::struct_type )
	{
		if( is_member && !enable_nested ){
			return false;
		}

		// TODO: do not support nested aggregated variable. 
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

reflection_impl_ptr reflect(module_semantic_ptr const& sem)
{
	reflector rfl;
	return rfl.reflect(sem);
}

reflection_impl_ptr reflect(module_semantic_ptr const& sem, eflib::fixed_string const& entry_name)
{
	reflector rfl;
	return rfl.reflect(sem, entry_name);
}
END_NS_SASL_SEMANTIC();