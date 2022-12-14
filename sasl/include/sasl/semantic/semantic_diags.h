#pragma once

#include <sasl/common/diag_formatter.h>
#include <sasl/common/diag_item.h>
#include <sasl/semantic/semantic_forward.h>

namespace sasl::syntax_tree {
struct node;
struct tynode;
} // namespace sasl::syntax_tree

namespace sasl::semantic {

// struct list_repr_end{ ostream& operator << (ostream&){} };
//
//// Repr utilities.
// template <typename T, typename ListReprT = list_repr_end >
// class list_repr
//{
// public:
//    list_repr( T const& v, ListReprT const& tail ): v(v), tail(tail){}
//    typedef list_repr<T, ListReprT> this_type;
//
//    template <typename U>
//    list_repr<U, this_type>& operator % ( U const& v )
//    {
//        return list_repr( v, *this );
//    }
//
//    ostream& operator << ( ostream& ostr )
//    {
//        ostr << v << tail;
//        return ostr;
//    }
// private:
//    T            v;
//    ListReprT    tail;
// };

class args_type_repr {
public:
  args_type_repr();
  args_type_repr &arg(std::shared_ptr<sasl::syntax_tree::tynode> const &);
  args_type_repr &arg(sasl::syntax_tree::node *);
  std::string str();

private:
  std::vector<std::shared_ptr<sasl::syntax_tree::tynode>> arg_tys;
  std::string str_buffer;
};

class type_repr {
public:
  type_repr(std::shared_ptr<sasl::syntax_tree::tynode> const &);
  type_repr(sasl::syntax_tree::tynode *);
  std::string str();

private:
  std::shared_ptr<sasl::syntax_tree::tynode> ty;
  std::string str_cache;
};

class source_position_repr {
public:
  source_position_repr(std::shared_ptr<sasl::common::token_t> const &beg,
                       std::shared_ptr<sasl::common::token_t> const &end,
                       sasl::common::compiler_compatibility cc);
  std::string str();

private:
  std::shared_ptr<sasl::common::token_t> beg, end;
  sasl::common::compiler_compatibility cc;
  std::string str_cache;
};

using sasl::common::diag_levels;
using sasl::common::diag_template;

constexpr diag_template unknown_semantic_error      {2000, diag_levels::fatal_error, "unknown semantic error occurred on '{}':{:d}"};
constexpr diag_template function_arg_count_error    {2001, diag_levels::error, "'{}': no overloaded function takes {:d} arguments" };
constexpr diag_template function_param_unmatched    {2002, diag_levels::error, "'{}': no overloaded function could convert all argument types\n\twhile trying to match '{}'" };
constexpr diag_template function_multi_overloads    {2003, diag_levels::error, "'{}': {:d} overloads have similar conversations." };
constexpr diag_template not_a_member_of             {2004, diag_levels::error, "'{}': not a member of '{}'" };
constexpr diag_template invalid_swizzle             {2005, diag_levels::error, "'{}': invalid swizzle of '{}'." };
constexpr diag_template operator_param_unmatched    {2006, diag_levels::error, "no overloaded operator could convert all argument types\n\twhile trying to match '{}'" };
constexpr diag_template operator_multi_overloads    {2007, diag_levels::error, "{:d} overloads have similar conversations." };
constexpr diag_template member_left_must_have_struct{2008, diag_levels::error, "left of '.{}' must have struct\n\ttype is '{}'"};
constexpr diag_template cannot_convert_type_from    {2009, diag_levels::error, "'{}': cannot convert from '{}' to '{}'"};
constexpr diag_template illegal_use_type_as_expr    {2010, diag_levels::error, "'{}': illegal use of this type as an expression" };
constexpr diag_template undeclared_identifier       {2011, diag_levels::error, "'{}': undeclared identifier"};
constexpr diag_template type_redefinition           {2012, diag_levels::error, "'{}': '{}' type redefinition"};
constexpr diag_template case_expr_not_constant      {2013, diag_levels::error, "case expression not constant"};
constexpr diag_template illegal_type_for_case_expr  {2014, diag_levels::error, "'{}': illegal type for case expression"};
constexpr diag_template identifier_not_found        {2015, diag_levels::error, "'{identifier:s}': identifier not found"};
constexpr diag_template not_an_acceptable_operator  {2016, diag_levels::error, "binary '{}': '{}' is not acceptable to the predefined operator"};
constexpr diag_template subscript_not_integral      {2017, diag_levels::error, "subscript is not of integral type"};
constexpr diag_template left_operand_must_be_lvalue {2018, diag_levels::error, "'{}': left operand must be l-value"};
constexpr diag_template operator_needs_lvalue       {2019, diag_levels::error, "'{:s}' needs l-value"};
constexpr diag_template not_support_auto_semantic   {2020, diag_levels::error, "Semantic verified failed. Note: Current version didn't support semantics propagates from parent to members"};

} // namespace sasl::semantic
