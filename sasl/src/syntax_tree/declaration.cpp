#include <sasl/syntax_tree/declaration.h>
#include <sasl/syntax_tree/node.h>
#include <sasl/syntax_tree/statement.h>
#include <sasl/syntax_tree/visitor.h>

#include <eflib/utility/enum.h>

using namespace eflib::enum_operators;

using std::shared_ptr;

namespace sasl::syntax_tree {

bool tynode::is_builtin() const { return node_class() == node_ids::builtin_type; }

bool tynode::is_struct() const { return node_class() == node_ids::struct_type; }

bool tynode::is_array() const { return node_class() == node_ids::array_type; }

bool tynode::is_function() const { return node_class() == node_ids::function_full_def; }

bool tynode::is_alias() const { return node_class() == node_ids::alias_type; }

bool tynode::is_uniform() const { return eflib::e2i(qual & type_qualifiers::_uniform) != 0; }

bool function_full_def::declaration_only() { return body || body->stmts.empty(); }

bool function_def::declaration_only() { return body || body->stmts.empty(); }
} // namespace sasl::syntax_tree