#pragma once

#include <memory>
#include <vector>

namespace salvia::core {
class renderer;
}

namespace salvia::resource {
using texture_ptr = std::shared_ptr<class texture>;
}

namespace salvia::ext::resource {

void make_terrain_random(std::vector<float>& field, int size);
void make_terrain_plasma(std::vector<float>& field, int size, float rough);
void make_terrain_fault(std::vector<float>& field,
                        int size,
                        int iterations,
                        int max_delta,
                        int min_delta,
                        int iterations_per_filter,
                        float filter);

void filter_terrain(std::vector<float>& field, int size, float filter);

salvia::resource::texture_ptr make_terrain_texture(salvia::core::renderer* render,
                                                   std::vector<float>& normalized_field,
                                                   size_t size);

}  // namespace salvia::ext::resource