#include <sasl/include/semantic/semantic_diags.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sstream>

using sasl::common::diag_template;
using sasl::common::dl_error;
using sasl::syntax_tree::tynode;
using sasl::syntax_tree::struct_type;
using sasl::syntax_tree::function_type;

using boost::shared_ptr;

using std::string;
using std::stringstream;

BEGIN_NS_SASL_SEMANTIC();

diag_template function_arg_count_error( dl_error, "'%s': no overloaded function takes %d arguments" );
diag_template function_param_unmatched( dl_error, "'%s': no overloaded function could convert all argument types\n\t while trying to match '%s'" );
diag_template function_multi_overloads( dl_error, "'%s': %d overloads have similar conversations." );
diag_template not_a_member_of( dl_error, "'%s': not a member of '%s'" );
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

args_type_repr& args_type_repr::arg( shared_ptr<tynode> const& arg_ty )
{
	arg_tys.push_back( arg_ty );
	return *this;
}

string args_type_repr::str()
{
	if( str_buffer.empty() )
	{
		stringstream sstr;

		if( arg_tys.empty() )
		{
			sstr << "( )";
		}
		else
		{
			sstr << "(" << type_repr(arg_tys[0]).str();
			for( size_t i = 1; i < arg_tys.size(); ++i )
			{
				sstr << " ,"<< type_repr(arg_tys[i]).str();
			}
			sstr << ")";
		}

		str_buffer = sstr.str();
	}

	return str_buffer;
} 

args_type_repr::args_type_repr()
{

}

END_NS_SASL_SEMANTIC();
