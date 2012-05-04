#include <sasl/include/semantic/semantic_diags.h>

#include <sasl/include/syntax_tree/declaration.h>

#include <sasl/enums/enums_utility.h>
#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sstream>

using sasl::utility::scalar_of;
using sasl::utility::is_vector;
using sasl::utility::is_matrix;
using sasl::utility::vector_count;
using sasl::utility::vector_size;

using sasl::common::diag_template;
using sasl::common::dl_error;
using sasl::common::dl_fatal_error;
using sasl::common::token_t;
using sasl::common::compiler_compatibility;
using sasl::syntax_tree::tynode;
using sasl::syntax_tree::struct_type;
using sasl::syntax_tree::function_type;

using boost::shared_ptr;

using std::string;
using std::stringstream;

BEGIN_NS_SASL_SEMANTIC();

diag_template unknown_semantic_error(dl_fatal_error, "unknown semantic error occurred on '%s':%d");
diag_template function_arg_count_error( dl_error, "'%s': no overloaded function takes %d arguments" );
diag_template function_param_unmatched( dl_error, "'%s': no overloaded function could convert all argument types\n\twhile trying to match '%s'" );
diag_template function_multi_overloads( dl_error, "'%s': %d overloads have similar conversations." );
diag_template not_a_member_of( dl_error, "'%s': not a member of '%s'" );
diag_template invalid_swizzle( dl_error, "'%s': invalid swizzle of '%s'." );
diag_template operator_param_unmatched( dl_error, "no overloaded operator could convert all argument types\n\twhile trying to match '%s'" );
diag_template operator_multi_overloads( dl_error, "%d overloads have similar conversations." );
diag_template member_left_must_have_struct( dl_error, "left of '.%s' must have struct\n\ttype is '%s'");
diag_template cannot_convert_type_from( dl_error, "'%s': cannot convert from '%s' to '%s'");
diag_template illegal_use_type_as_expr( dl_error, "'%s': illegal use of this type as an expression" );
diag_template undeclared_identifier(dl_error, "'%s': undeclared identifier");
diag_template type_redefinition(dl_error, "'%s': '%s' type redefinition");
diag_template case_expr_not_constant(dl_error, "case expression not constant");
diag_template illegal_type_for_case_expr(dl_error, "'%s': illegal type for case expression");
diag_template identifier_not_found(dl_error, "'%s': identifier not found");
diag_template not_an_acceptable_operator(dl_error, "binary '%s': '%s' is not acceptable to the predefined operator");
diag_template subscript_not_integral(dl_error, "subscript is not of integral type");

char const* scalar_nick_name( builtin_types btcode )
{
	if( btcode == builtin_types::_sint8 ) {
		return "char";
	} else if ( btcode == builtin_types::_uint8 ) {
		return "byte";
	} else if ( btcode == builtin_types::_sint16 ) {
		return "short";
	} else if ( btcode == builtin_types::_uint16 ) {
		return "ushort";
	} else if ( btcode == builtin_types::_sint32 ) {
		return "int";
	} else if ( btcode == builtin_types::_uint32 ) {
		return "uint";
	} else if ( btcode == builtin_types::_sint64 ) {
		return "long";
	} else if ( btcode == builtin_types::_uint64 ) {
		return "ulong";
	} else if ( btcode == builtin_types::_float ) {
		return "float";
	} else if ( btcode == builtin_types::_double ) {
		return "double";
	} else if ( btcode == builtin_types::none ) {
		return "none";
	} else if ( btcode == builtin_types::_sampler ) {
		return "sampler";
	} else if ( btcode == builtin_types::_boolean ) {
		return "bool";
	} else {
		assert(false);
		return "<unknown>";
	}
}

type_repr::type_repr( shared_ptr<tynode> const& ty ): ty(ty)
{
}

string type_repr::str()
{
	if( str_cache.empty() )
	{
		if( !ty )
		{
			str_cache = "<unknown>";
		}
		else if( ty->is_builtin() )
		{
			stringstream name_stream;

			builtin_types bt_code		= ty->tycode;
			builtin_types scalar_code	= scalar_of(bt_code);
			char const* scalar_name = scalar_nick_name(scalar_code);
			if( is_matrix( bt_code ) )
			{
				name_stream << scalar_name << vector_count(bt_code) << "x" << vector_size(bt_code);
			}
			else if( is_vector(bt_code) )
			{
				name_stream << scalar_name << vector_size(bt_code);
			}
			else
			{
				name_stream << scalar_nick_name(bt_code);
			}
			std::string name = name_stream.str();
			str_cache.assign( name.begin(), name.end() );
		}
		else if ( ty->is_struct() )
		{
			str_cache = "struct ";
			str_cache += ty->as_handle<struct_type>()->name->str;
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
				sstr << ", "<< type_repr(arg_tys[i]).str();
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

source_position_repr::source_position_repr(
	shared_ptr<token_t> const& beg, shared_ptr<token_t> const& end,
	compiler_compatibility cc ): beg(beg), end(end), cc(cc)
{
	
}

std::string source_position_repr::str()
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return std::string();
}

END_NS_SASL_SEMANTIC();
