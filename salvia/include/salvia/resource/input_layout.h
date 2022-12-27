#pragma once

#include "salvia/common/format.h"

#include <salvia/shader/constants.h>

#include <eflib/utility/shared_declaration.h>

#include <vector>

namespace salvia::resource {

enum input_classifications {
  input_per_vertex

  // TODO:
  // input_per_instance
};

struct input_element_desc {
  std::string semantic_name;
  uint32_t semantic_index;
  format data_format;
  uint32_t input_slot;
  uint32_t aligned_byte_offset;
  input_classifications slot_class;

  uint32_t instance_data_step_rate;

  input_element_desc(const char *semantic_name, uint32_t semantic_index, format data_format,
                     uint32_t input_slot, uint32_t aligned_byte_offset,
                     input_classifications slot_class, uint32_t instance_data_step_rate)
      : semantic_name(semantic_name), semantic_index(semantic_index), data_format(data_format),
        input_slot(input_slot), aligned_byte_offset(aligned_byte_offset), slot_class(slot_class),
        instance_data_step_rate(instance_data_step_rate) {}

  input_element_desc()
      : semantic_index(0), data_format(format_unknown), input_slot(0),
        aligned_byte_offset(0xFFFFFFFF), slot_class(input_per_vertex), instance_data_step_rate(0) {}
};

EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);

class input_layout {
private:
  using semantic_value = salvia::shader::semantic_value;

public:
  static input_layout_ptr create(input_element_desc const *pdesc, size_t desc_count);
  virtual ~input_layout() {}
  typedef std::vector<input_element_desc>::const_iterator iterator;

  iterator desc_begin() const;
  iterator desc_end() const;

  semantic_value get_semantic(iterator it) const;

  size_t find_slot(semantic_value const &) const;

  virtual input_element_desc const *find_desc(size_t slot) const;
  virtual input_element_desc const *find_desc(semantic_value const &) const;

  void slot_range(size_t &min_slot, size_t &max_slot) const;

private:
  std::vector<input_element_desc> descs;
};

size_t hash_value(input_element_desc const &);
size_t hash_value(input_layout const &);

} // namespace salvia::resource

namespace std {
template <> struct hash<salvia::resource::input_element_desc> {
  size_t operator()(salvia::resource::input_element_desc const &v) const noexcept {
    return salvia::resource::hash_value(v);
  }
};

template <> struct hash<salvia::resource::input_layout> {
  size_t operator()(salvia::resource::input_layout const &v) const noexcept {
    return salvia::resource::hash_value(v);
  }
};
} // namespace std