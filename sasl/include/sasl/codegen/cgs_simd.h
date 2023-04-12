#pragma once

#include <sasl/codegen/forward.h>

#include <sasl/codegen/cgs.h>

namespace sasl::codegen {

class cgs_simd : public cg_service {
public:
  cgs_simd();

  virtual void store(multi_value& lhs, multi_value const& rhs) override;

  multi_value emit_and(multi_value const& lhs, multi_value const& rhs) override;
  multi_value emit_or(multi_value const& lhs, multi_value const& rhs) override;

  virtual multi_value cast_ints(multi_value const& v, cg_type* dest_tyi) override;
  virtual multi_value cast_i2f(multi_value const& v, cg_type* dest_tyi) override;
  virtual multi_value cast_f2i(multi_value const& v, cg_type* dest_tyi) override;
  virtual multi_value cast_f2f(multi_value const& v, cg_type* dest_tyi) override;
  virtual multi_value cast_i2b(multi_value const& v) override;
  virtual multi_value cast_f2b(multi_value const& v) override;

  void emit_return() override;
  void emit_return(multi_value const&, abis abi) override;

  virtual multi_value emit_ddx(multi_value const& v) override;
  virtual multi_value emit_ddy(multi_value const& v) override;

  virtual multi_value create_vector(std::vector<multi_value> const& scalars, abis abi) override;

  bool prefer_externals() const override { return false; }
  bool prefer_scalar_code() const override { return false; }
  llvm::Value* current_execution_mask() const override;

  virtual void function_body_beg() override;
  virtual void function_body_end() override;

  virtual void for_init_beg() override;
  virtual void for_init_end() override;
  virtual void for_cond_beg() override;
  virtual void for_cond_end(multi_value const&) override;
  virtual void for_body_beg() override;
  virtual void for_body_end() override;
  virtual void for_iter_beg() override;
  virtual void for_iter_end() override;

  virtual multi_value any_mask_true() override;

  virtual void if_beg() override;
  virtual void if_end() override;
  virtual void if_cond_beg() override;
  virtual void if_cond_end(multi_value const&) override;
  virtual void then_beg() override;
  virtual void then_end() override;
  virtual void else_beg() override;
  virtual void else_end() override;

  virtual void switch_cond_beg() override {}
  virtual void switch_cond_end() override {}
  virtual void switch_expr_beg() override {}
  virtual void switch_expr_end() override {}

  virtual void while_beg() override;
  virtual void while_end() override;
  virtual void while_cond_beg() override;
  virtual void while_cond_end(multi_value const&) override;
  virtual void while_body_beg() override;
  virtual void while_body_end() override;

  virtual void do_beg() override;
  virtual void do_end() override;
  virtual void do_body_beg() override;
  virtual void do_body_end() override;
  virtual void do_cond_beg() override;
  virtual void do_cond_end(multi_value const&) override;

  virtual void break_() override;
  virtual void continue_() override;

private:
  llvm::Value* load_loop_execution_mask();
  void save_loop_execution_mask(llvm::Value* mask = nullptr);
  virtual void enter_loop();
  virtual void exit_loop();
  virtual void apply_loop_condition(multi_value const&);
  virtual void save_next_iteration_exec_mask();

  llvm::Value* all_one_mask();
  llvm::Value* all_zero_mask();

  enum derivation_directional { dd_horizontal, dd_vertical };

  multi_value derivation(multi_value const& v, derivation_directional dd);

  // Apply break mask and continue mask to top of exec mask stack.
  //  It affects the following execution.
  void apply_break_and_continue();
  void apply_break();
  void apply_continue();

  // Masks
  std::vector<llvm::Value*> cond_exec_masks;
  std::vector<llvm::Value*> mask_vars;
  std::vector<llvm::Value*> break_masks;
  std::vector<llvm::Value*> continue_masks;
  std::vector<llvm::Value*> exec_masks;
};

}  // namespace sasl::codegen