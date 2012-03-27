#include <sasl/include/semantic/semantic_diags.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <eflib/include/diagnostics/assert.h>

using sasl::common::diag_template;
using sasl::common::dl_error;

using sasl::syntax_tree::tynode;
using sasl::syntax_tree::struct_type;
using sasl::syntax_tree::function_type;
using boost::shared_ptr;
using std::string;

BEGIN_NS_SASL_SEMANTIC();

type_repr::type_repr( shared_ptr<tynode> const& ty ): ty(ty)
{
}

string type_repr::str()
{
	if( str_cache.empty() )
	{
		if( ty->is_builtin() )
		{
			string name = builtin_types::to_name(ty->tycode);
			str_cache.assign( name.begin()+1, name.end() );
		}
		else if ( ty->is_struct() )
		{
			str_cache = ty->as_handle<struct_type>()->name->str;
		}
		else if ( ty->is_function() )
		{
			shared_ptr<function_type> fn = ty->as_handle<function_type>();
			str_cache = type_repr(fn->retval_type).str();
			str_cache += " ";
			str_cache += fn->name->str;
			str_cache += "(";
			if( !fn->params.empty() )
			{
				for( size_t i = 1; i < fn->params.size(); ++i )
				{
					str_cache += ", ";
					str_cache += type_repr(fn->params[i]->param_type).str();
				}
			}
			str_cache += ");";
		}
		else
		{
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}
	return str_cache;
}

diag_template function_arg_count_error( dl_error, "'%s': no overloaded function takes %d arguments" );
diag_template function_param_unmatched( dl_error, "'%s': no overloaded function could convert all argument types. \r\n\t while trying to match '%s'" );

END_NS_SASL_SEMANTIC();
