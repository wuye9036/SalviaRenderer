#pragma once

#if defined(sasl_host_EXPORTS)
#define SALVIA_API __declspec(dllexport)
#else
#define SALVIA_API __declspec(dllimport)
#endif

#include <salviar/include/decl.h>

#include <eflib/memory/allocator.h>
#include <eflib/platform/typedefs.h>
#include <eflib/utility/shared_declaration.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace salviar {

class shader_reflection;
class shader_object;
class vs_output;
struct ps_output;
class stream_assembler;

class pixel_shader_unit {
public:
  pixel_shader_unit();
  ~pixel_shader_unit();

  pixel_shader_unit(pixel_shader_unit const &);
  pixel_shader_unit &operator=(pixel_shader_unit const &);

  std::shared_ptr<pixel_shader_unit> clone() const;

  void initialize(shader_object const *);
  void reset_pointers();

  void set_variable(std::string const &, void const *data);
  void set_sampler(std::string const &, sampler_ptr const &samp);

  void update(vs_output *inputs, shader_reflection const *vs_abi);
  void execute(ps_output *outs, float *depths);

public:
  shader_object const *code;

  std::vector<sampler_ptr> used_samplers; // For take ownership

  typedef std::vector<char, eflib::aligned_allocator<char, 32>> aligned_vector;

  aligned_vector stream_data;
  aligned_vector buffer_data;

  aligned_vector stream_odata;
  aligned_vector buffer_odata;
};

EFLIB_DECLARE_CLASS_SHARED_PTR(vx_shader_unit);
class vx_shader_unit {
public:
  virtual uint32_t output_attributes_count() const = 0;
  virtual uint32_t output_attribute_modifiers(size_t index) const = 0;

  virtual void execute(size_t ivert, void *out_data) = 0;
  virtual void execute(size_t ivert, vs_output &out) = 0;

  virtual ~vx_shader_unit() {}
};

} // namespace salviar