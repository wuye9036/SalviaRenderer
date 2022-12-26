#pragma once

#include <eflib/math/vector.h>

namespace salvia::resource {

class sampler;

void tex2Dlod(eflib::vec4 &result, sampler *samp, eflib::vec4 &coord);
void texCUBElod(eflib::vec4 &result, sampler *samp, eflib::vec4 &coord);

void tex2Dgrad_ps(eflib::vec4 *results, uint32_t mask, sampler *samp, eflib::vec2 *coords,
                  eflib::vec2 const *ddxs, eflib::vec2 const *ddys);
void tex2Dbias_ps(eflib::vec4 *results, uint32_t mask, sampler *samp, eflib::vec4 *coords,
                  eflib::vec2 const *ddxs, eflib::vec2 const *ddys);
void tex2Dlod_ps(eflib::vec4 *results, uint32_t mask, sampler *samp, eflib::vec4 *coords);
void tex2Dproj_ps(eflib::vec4 *results, uint32_t mask, sampler *samp, eflib::vec4 *coords,
                  eflib::vec4 const *ddxs, eflib::vec4 const *ddys);

} // namespace salvia::resource