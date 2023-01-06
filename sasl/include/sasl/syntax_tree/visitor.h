#pragma once

#include <sasl/syntax_tree/syntax_tree_fwd.h>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <eflib/diagnostics/assert.h>

#include <any>

namespace sasl::syntax_tree {

struct unary_expression;
struct cast_expression;
struct expression_list;
struct cond_expression;
struct index_expression;
struct call_expression;
struct member_expression;
struct variable_expression;
struct identifier;
struct constant_expression;
struct binary_expression;

struct initializer;
struct expression_initializer;
struct member_initializer;
struct declaration;
struct declarator;
struct variable_declaration;
struct type_definition;
struct tynode;
struct builtin_type;
struct array_type;
struct struct_type;
struct alias_type;
struct parameter_full;
struct function_full_def;
struct parameter;
struct function_type;
struct function_def;

struct statement;
struct declaration_statement;
struct if_statement;
struct while_statement;
struct dowhile_statement;
struct for_statement;
struct label;
struct case_label;
struct ident_label;
struct switch_statement;
struct compound_statement;
struct expression_statement;
struct jump_statement;
struct labeled_statement;

struct program;

// Macros for making define visitor shortly.

// For expand SASL_VISIT_DEF_UNIMPL_ and SASL_VISIT_DEF
// #define SASL_VISITOR_TYPE_NAME
//

// It will be expand as a function as following:
//	void visit ( ... ){
//		EFLIB_ASSERT_UNIMPLEMENTED0( "XXX::visit was not implemented yet." );
//	}
#define SASL_VISIT_DEF_UNIMPL(node_type_name) SASL_SPECIFIC_VISIT_DEF_UNIMPL(visit, node_type_name)
#define SASL_SPECIFIC_VISIT_DEF_UNIMPL(fn, node_type_name)                                         \
  void SASL_VISITOR_TYPE_NAME::fn(::sasl::syntax_tree::node_type_name &, ::std::any *) {           \
    EFLIB_ASSERT_UNIMPLEMENTED0(                                                                   \
        (::std::string(BOOST_PP_STRINGIZE( SASL_VISITOR_TYPE_NAME::node_type_name ) ) +            \
                       ::std::string(" was not implemented yet."))                                 \
             .c_str());                                                                            \
  }

#define SASL_VISIT_DEF(node_type_name) SASL_SPECIFIC_VISIT_DEF(visit, node_type_name)
#define SASL_SPECIFIC_VISIT_DEF(fn, node_type_name)                                                \
  void SASL_VISITOR_TYPE_NAME::fn(::sasl::syntax_tree::node_type_name &v, ::std::any *data)

#define SASL_VISIT_DCL(node_type_name) SASL_SPECIFIC_VISIT_DCL(visit, node_type_name) override
#define SASL_SPECIFIC_VISIT_DCL(fn, node_type_name)                                                \
  virtual void fn(::sasl::syntax_tree::node_type_name &v, ::std::any *data = nullptr)

#define SASL_VISIT_BASE_DCL(node_type_name)                                                        \
  virtual void visit(::sasl::syntax_tree::node_type_name &v, ::std::any *data = nullptr);

#define SASL_VISIT_ABSTRACT_DCL(node_type_name)                                                    \
  virtual void visit(::sasl::syntax_tree::node_type_name &v, ::std::any *data = nullptr) = 0;

#define SASL_VISIT_INLINE_DEF_UNIMPL(node_type_name)                                               \
  SASL_SPECIFIC_VISIT_INLINE_DEF_UNIMPL(visit, node_type_name)
#define SASL_SPECIFIC_VISIT_INLINE_DEF_UNIMPL(fn, node_type_name)                                  \
  virtual void fn(::sasl::syntax_tree::node_type_name &, ::std::any * = nullptr) override {        \
    EFLIB_ASSERT_UNIMPLEMENTED0((::std::string(BOOST_PP_STRINGIZE( node_type_name ) ) +            \
                                               ::std::string(" was not implemented yet."))         \
                                     .c_str());                                                    \
  }

class syntax_tree_visitor {
public:
  // expression
  SASL_VISIT_ABSTRACT_DCL(unary_expression);
  SASL_VISIT_ABSTRACT_DCL(cast_expression);
  SASL_VISIT_ABSTRACT_DCL(binary_expression);
  SASL_VISIT_ABSTRACT_DCL(expression_list);
  SASL_VISIT_ABSTRACT_DCL(cond_expression);
  SASL_VISIT_ABSTRACT_DCL(index_expression);
  SASL_VISIT_ABSTRACT_DCL(call_expression);
  SASL_VISIT_ABSTRACT_DCL(member_expression);
  SASL_VISIT_ABSTRACT_DCL(constant_expression);
  SASL_VISIT_ABSTRACT_DCL(variable_expression);

  // declaration & type specifier
  SASL_VISIT_ABSTRACT_DCL(initializer);
  SASL_VISIT_ABSTRACT_DCL(expression_initializer);
  SASL_VISIT_ABSTRACT_DCL(member_initializer);
  SASL_VISIT_ABSTRACT_DCL(declaration);
  SASL_VISIT_ABSTRACT_DCL(declarator);
  SASL_VISIT_ABSTRACT_DCL(variable_declaration);
  SASL_VISIT_ABSTRACT_DCL(type_definition);
  SASL_VISIT_ABSTRACT_DCL(function_def);
  SASL_VISIT_ABSTRACT_DCL(parameter);
  SASL_VISIT_ABSTRACT_DCL(function_full_def);
  SASL_VISIT_ABSTRACT_DCL(parameter_full);
  SASL_VISIT_ABSTRACT_DCL(tynode);
  SASL_VISIT_ABSTRACT_DCL(builtin_type);
  SASL_VISIT_ABSTRACT_DCL(array_type);
  SASL_VISIT_ABSTRACT_DCL(struct_type);
  SASL_VISIT_ABSTRACT_DCL(alias_type);
  SASL_VISIT_ABSTRACT_DCL(function_type);

  // statement
  SASL_VISIT_ABSTRACT_DCL(statement);
  SASL_VISIT_ABSTRACT_DCL(declaration_statement);
  SASL_VISIT_ABSTRACT_DCL(if_statement);
  SASL_VISIT_ABSTRACT_DCL(while_statement);
  SASL_VISIT_ABSTRACT_DCL(dowhile_statement);
  SASL_VISIT_ABSTRACT_DCL(for_statement);
  SASL_VISIT_ABSTRACT_DCL(case_label);
  SASL_VISIT_ABSTRACT_DCL(ident_label);
  SASL_VISIT_ABSTRACT_DCL(switch_statement);
  SASL_VISIT_ABSTRACT_DCL(compound_statement);
  SASL_VISIT_ABSTRACT_DCL(expression_statement);
  SASL_VISIT_ABSTRACT_DCL(jump_statement);
  SASL_VISIT_ABSTRACT_DCL(labeled_statement);

  // program
  SASL_VISIT_ABSTRACT_DCL(program);

  template <typename NodeT> void visit(NodeT &, std::any *) {
    ef_unreachable("this function would never be called.");
  }

  virtual ~syntax_tree_visitor(){};
};

template <typename Visitor> class syntax_tree_static_visitor : public syntax_tree_visitor {
public:
  syntax_tree_static_visitor(Visitor const &v) : vis_{v} {}
  syntax_tree_static_visitor(Visitor &&v) : vis_{std::move(v)} {}

  // expression
  SASL_VISIT_DCL(unary_expression) { vis_(v, data); }
  SASL_VISIT_DCL(cast_expression) { vis_(v, data); }
  SASL_VISIT_DCL(binary_expression) { vis_(v, data); }
  SASL_VISIT_DCL(expression_list) { vis_(v, data); }
  SASL_VISIT_DCL(cond_expression) { vis_(v, data); }
  SASL_VISIT_DCL(index_expression) { vis_(v, data); }
  SASL_VISIT_DCL(call_expression) { vis_(v, data); }
  SASL_VISIT_DCL(member_expression) { vis_(v, data); }
  SASL_VISIT_DCL(constant_expression) { vis_(v, data); }
  SASL_VISIT_DCL(variable_expression) { vis_(v, data); }

  // declaration & type specifier
  SASL_VISIT_DCL(initializer) { vis_(v, data); }
  SASL_VISIT_DCL(expression_initializer) { vis_(v, data); }
  SASL_VISIT_DCL(member_initializer) { vis_(v, data); }
  SASL_VISIT_DCL(declaration) { vis_(v, data); }
  SASL_VISIT_DCL(declarator) { vis_(v, data); }
  SASL_VISIT_DCL(variable_declaration) { vis_(v, data); }
  SASL_VISIT_DCL(type_definition) { vis_(v, data); }
  SASL_VISIT_DCL(function_def) { vis_(v, data); }
  SASL_VISIT_DCL(parameter) { vis_(v, data); }
  SASL_VISIT_DCL(function_full_def) { vis_(v, data); }
  SASL_VISIT_DCL(parameter_full) { vis_(v, data); }
  SASL_VISIT_DCL(tynode) { vis_(v, data); }
  SASL_VISIT_DCL(builtin_type) { vis_(v, data); }
  SASL_VISIT_DCL(array_type) { vis_(v, data); }
  SASL_VISIT_DCL(struct_type) { vis_(v, data); }
  SASL_VISIT_DCL(alias_type) { vis_(v, data); }
  SASL_VISIT_DCL(function_type) { vis_(v, data); }

  // statement
  SASL_VISIT_DCL(statement) { vis_(v, data); }
  SASL_VISIT_DCL(declaration_statement) { vis_(v, data); }
  SASL_VISIT_DCL(if_statement) { vis_(v, data); }
  SASL_VISIT_DCL(while_statement) { vis_(v, data); }
  SASL_VISIT_DCL(dowhile_statement) { vis_(v, data); }
  SASL_VISIT_DCL(for_statement) { vis_(v, data); }
  SASL_VISIT_DCL(case_label) { vis_(v, data); }
  SASL_VISIT_DCL(ident_label) { vis_(v, data); }
  SASL_VISIT_DCL(switch_statement) { vis_(v, data); }
  SASL_VISIT_DCL(compound_statement) { vis_(v, data); }
  SASL_VISIT_DCL(expression_statement) { vis_(v, data); }
  SASL_VISIT_DCL(jump_statement) { vis_(v, data); }
  SASL_VISIT_DCL(labeled_statement) { vis_(v, data); }

  // program
  SASL_VISIT_DCL(program) { vis_(v, data); }

  Visitor vis_;
};

} // namespace sasl::syntax_tree