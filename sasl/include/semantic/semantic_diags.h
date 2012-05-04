#ifndef SASL_SEMANTIC_SEMANTIC_ERROR_H
#define SASL_SEMANTIC_SEMANTIC_ERROR_H

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/include/common/diag_item.h>
#include <sasl/include/common/diag_formatter.h>

namespace sasl
{
	namespace syntax_tree
	{
		struct node;
		struct tynode;
	}
}

BEGIN_NS_SASL_SEMANTIC();

//struct list_repr_end{ ostream& operator << (ostream&){} };
//
//// Repr utilities.
//template <typename T, typename ListReprT = list_repr_end >
//class list_repr
//{
//public:
//	list_repr( T const& v, ListReprT const& tail ): v(v), tail(tail){}
//	typedef list_repr<T, ListReprT> this_type;
//
//	template <typename U>
//	list_repr<U, this_type>& operator % ( U const& v )
//	{
//		return list_repr( v, *this );
//	}
//
//	ostream& operator << ( ostream& ostr )
//	{
//		ostr << v << tail;
//		return ostr;
//	}
//private:
//	T			v;
//	ListReprT	tail;
//};

class args_type_repr
{
public:
	args_type_repr();
	args_type_repr& arg( boost::shared_ptr<sasl::syntax_tree::tynode> const& );
	std::string str();
private:
	std::vector< boost::shared_ptr<sasl::syntax_tree::tynode> > arg_tys;
	std::string str_buffer;
};

class type_repr
{
public:
	type_repr( boost::shared_ptr<sasl::syntax_tree::tynode> const& );
	std::string str();
private:
	boost::shared_ptr<sasl::syntax_tree::tynode> ty;
	std::string str_cache;
};

class source_position_repr
{
public:
	source_position_repr(
		boost::shared_ptr<sasl::common::token_t> const& beg,
		boost::shared_ptr<sasl::common::token_t> const& end,
		sasl::common::compiler_compatibility cc );
	std::string str();
private:
	boost::shared_ptr<sasl::common::token_t> beg, end;
	sasl::common::compiler_compatibility cc;
	std::string str_cache;
};

extern sasl::common::diag_template unknown_semantic_error;
extern sasl::common::diag_template function_arg_count_error;
extern sasl::common::diag_template function_param_unmatched;
extern sasl::common::diag_template function_multi_overloads;
extern sasl::common::diag_template operator_param_unmatched;
extern sasl::common::diag_template operator_multi_overloads;
extern sasl::common::diag_template not_a_member_of;
extern sasl::common::diag_template invalid_swizzle;
extern sasl::common::diag_template member_left_must_have_struct;
extern sasl::common::diag_template cannot_convert_type_from;
extern sasl::common::diag_template illegal_use_type_as_expr;
extern sasl::common::diag_template undeclared_identifier;
extern sasl::common::diag_template identifier_not_found;
extern sasl::common::diag_template type_redefinition;
extern sasl::common::diag_template case_expr_not_constant;
extern sasl::common::diag_template illegal_type_for_case_expr;
extern sasl::common::diag_template not_an_acceptable_operator;
extern sasl::common::diag_template subscript_not_integral;

END_NS_SASL_SEMANTIC();

#endif
