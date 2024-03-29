#ifndef SASL_SEMANTIC_TYPE_MANAGER_H
#define SASL_SEMANTIC_TYPE_MANAGER_H

#include <sasl/semantic/semantic_forward.h>

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/operators.h>

#include <eflib/platform/stdint.h>
#include <eflib/utility/shared_declaration.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sasl {
namespace syntax_tree {
EFLIB_DECLARE_STRUCT_SHARED_PTR(tynode);
}
}  // namespace sasl

namespace sasl::semantic {

class module_semantic;
class node_semantic;
EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);
EFLIB_DECLARE_CLASS_SHARED_PTR(pety_t);

typedef int tid_t;

class pety_t {
public:
  static pety_t_ptr create(module_semantic* owner);

  virtual void root_symbol(symbol* sym) = 0;

  // Get TID from type informations.
  virtual tid_t get(const builtin_types& btc) = 0;
  virtual tid_t get(sasl::syntax_tree::tynode*, symbol*) = 0;
  virtual tid_t get_array(tid_t elem_type, size_t dimension) = 0;
  virtual tid_t get_function_type(std::vector<tid_t> const& fn_tids) = 0;
  // Get proto or semantic from TID.
  virtual sasl::syntax_tree::tynode* get_proto(tid_t tid) = 0;
  virtual sasl::syntax_tree::tynode* get_proto_by_builtin(builtin_types bt) = 0;
  virtual void get2(tid_t tid,
                    sasl::syntax_tree::tynode**, /*output node	 */
                    node_semantic**              /*output semantic*/
                    ) = 0;
  virtual void get2(builtin_types btc,
                    sasl::syntax_tree::tynode**, /*output node	 */
                    node_semantic**              /*output semantic*/
                    ) = 0;

  // Name mangling
  virtual std::string mangle(std::string_view, tid_t tid) = 0;
  virtual std::string_view operator_name(operators const& op) = 0;

  virtual ~pety_t() {}
};

}  // namespace sasl::semantic

#endif