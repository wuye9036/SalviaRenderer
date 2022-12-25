#ifndef SASL_CODEGEN_CGS_SISD_H
#define SASL_CODEGEN_CGS_SISD_H

#include <sasl/codegen/forward.h>

#include <sasl/codegen/cg_intrins.h>
#include <sasl/codegen/cgs.h>
#include <sasl/enums/builtin_types.h>

#include <boost/preprocessor/for.hpp>
#include <boost/preprocessor/seq.hpp>
#include <eflib/platform/boost_begin.h>
#include <eflib/platform/boost_end.h>

// #include <eflib/utility/util.h>
#include <eflib/diagnostics/assert.h>
#include <eflib/utility/enable_if.h>

#include <any>
#include <functional>
#include <memory>
#include <vector>

namespace llvm {
class Argument;
class Function;
class Type;
class Value;
class LLVMContext;
class Module;
class BasicBlock;
class ConstantInt;
class ConstantVector;

class IRBuilderDefaultInserter;
template <typename T, typename Inserter> class IRBuilder;
class ConstantFolder;
using DefaultIRBuilder = IRBuilder<ConstantFolder, IRBuilderDefaultInserter>;
} // namespace llvm

namespace sasl {
namespace syntax_tree {
struct node;
struct tynode;
struct function_full_def;
} // namespace syntax_tree
} // namespace sasl

namespace sasl::codegen {

class llvm_intrin_cache;

class cgs_sisd : public cg_service {
public:
  cgs_sisd();

  /** @name Emit expressions
  Some simple overload-able operators such as '+' '-' '*' '/'
  will be implemented in 'cgv_*' classes in operator overload form.
  @{ */
  multi_value emit_cond_expr(multi_value cond, multi_value const &yes, multi_value const &no);

  /// Didn't support swizzle yet.
  multi_value emit_swizzle(multi_value const &vec, uint32_t mask);
  multi_value emit_write_mask(multi_value const &vec, uint32_t mask);
  /** @} */

  /// @name Emit type casts
  /// @{
  /// Cast between integer types.
  multi_value cast_ints(multi_value const &v, cg_type *dest_tyi) override;
  /// Cast integer to float.
  multi_value cast_i2f(multi_value const &v, cg_type *dest_tyi) override;
  /// Cast float to integer.
  multi_value cast_f2i(multi_value const &v, cg_type *dest_tyi) override;
  /// Cast between float types.
  multi_value cast_f2f(multi_value const &v, cg_type *dest_tyi) override;
  /// Cast integer to bool
  multi_value cast_i2b(multi_value const &v) override;
  /// Cast float to bool
  multi_value cast_f2b(multi_value const &v) override;
  /// @}

  /// @name Emit Declarations
  /// @{
  cg_function begin_fndecl();
  cg_function end_fndecl();
  /// @}

  /// @name Intrinsics
  /// @{
  virtual multi_value emit_ddx(multi_value const &v) override;
  virtual multi_value emit_ddy(multi_value const &v) override;
  /// @}

  /// @name Emit statement
  /// @{
  void emit_return() override;
  void emit_return(multi_value const &, abis abi) override;
  /// @}

  /// @name Emit assignment
  /// @{
  virtual void store(multi_value &lhs, multi_value const &rhs) override;
  /// @}

  /// @name Emit values
  /// @{
  template <typename T>
  multi_value create_constant_vector(T const *vals, size_t length, abis abi,
                                     EFLIB_ENABLE_IF_PRED1(is_integral, T));
  multi_value create_vector(std::vector<multi_value> const &scalars, abis abi) override;

  template <typename T>
  multi_value create_constant_matrix(T const *vals, size_t length, abis abi,
                                     EFLIB_ENABLE_IF_PRED1(is_integral, T));
  /// @}

  // virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( size_t tyid ) = 0;
  // virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( builtin_types bt ) = 0;

  /// @name Utilities
  /// @{
  /// Switch to blocks
  void switch_to(multi_value const &cond,
                 std::vector<std::pair<multi_value, insert_point_t>> const &cases,
                 insert_point_t const &default_branch);
  /// @}

  /// @name Bridges
  /// @{
  llvm::Value *select_(llvm::Value *cond, llvm::Value *yes, llvm::Value *no);
  llvm::Value *phi_(llvm::BasicBlock *b0, llvm::Value *v0, llvm::BasicBlock *b1, llvm::Value *v1);
  template <typename T>
  llvm::Value *c_vector_(T const *vals, size_t length, EFLIB_ENABLE_IF_PRED1(is_integral, T));
  /// @}

  /// @name State
  /// @{
  /// Prefer to use external functions as intrinsic.
  bool prefer_externals() const override;
  /// Prefer to use scalar code to intrinsic.
  bool prefer_scalar_code() const override;
  /// @}

  llvm::Value *current_execution_mask() const override{ return nullptr; }
};

} // namespace sasl::codegen

#endif