#pragma once

#if defined(sasl_host_EXPORTS)
#define SALVIA_API __declspec(dllexport)
#else
#define SALVIA_API __declspec(dllimport)
#endif

#include <salvia/core/decl.h>

#include <salvia/shader/shader_regs.h>
#include <salvia/shader/reflection.h>

#include <eflib/memory/allocator.h>
#include <eflib/platform/stdint.h>
#include <eflib/utility/shared_declaration.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace salvia::core {

class stream_assembler;

class pixel_shader_unit {
public:
  pixel_shader_unit();
  ~pixel_shader_unit();

  pixel_shader_unit(pixel_shader_unit const &);
  pixel_shader_unit &operator=(pixel_shader_unit const &);

  [[nodiscard]] std::shared_ptr<pixel_shader_unit> clone() const;

  void initialize(shader::shader_object const *);
  void reset_pointers();

  void set_variable(std::string const &, void const *data);
  void set_sampler(std::string const &, resource::sampler_ptr const &samp);

  void update(shader::vs_output *inputs, shader::shader_reflection const *vs_abi);
  void execute(shader::ps_output *outs, float *depths);

public:
  shader::shader_object const *code;

  std::vector<resource::sampler_ptr> used_samplers; // For take ownership

  typedef std::vector<char, eflib::aligned_allocator<char, 32>> aligned_vector;

  aligned_vector stream_data;
  aligned_vector buffer_data;

  aligned_vector stream_odata;
  aligned_vector buffer_odata;
};

EFLIB_DECLARE_CLASS_SHARED_PTR(vx_shader_unit);
class vx_shader_unit {
public:
  [[nodiscard]] virtual uint32_t output_attributes_count() const = 0;
  [[nodiscard]] virtual uint32_t output_attribute_modifiers(size_t index) const = 0;

  virtual void execute(size_t i_vertex, void *out_data) = 0;
  virtual void execute(size_t i_vertex, shader::vs_output &out) = 0;

  virtual ~vx_shader_unit() = default;
};

} // namespace salvia::core