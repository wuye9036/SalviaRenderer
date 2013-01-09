#ifndef SALVIAR_SAMPLER_API_H
#define SALVIAR_SAMPLER_API_H

#include <salviar/include/salviar_forward.h>

#include <eflib/include/math/vector.h>

BEGIN_NS_SALVIAR();

class sampler;

void tex2Dlod  (eflib::vec4& result, salviar::sampler* samp, eflib::vec4& coord);
void texCUBElod(eflib::vec4& result, salviar::sampler* samp, eflib::vec4& coord);

void tex2Dgrad_ps(
	eflib::vec4* results, uint32_t mask,
	salviar::sampler* samp, eflib::vec2* coords,
	eflib::vec2 const* ddxs, eflib::vec2 const* ddys );
void tex2Dbias_ps(
	eflib::vec4* results, uint32_t mask,
	salviar::sampler* samp, eflib::vec4* coords,
	eflib::vec2 const* ddxs, eflib::vec2 const* ddys );
void tex2Dlod_ps(
	eflib::vec4* results, uint32_t mask,
	salviar::sampler* samp, eflib::vec4* coords );
void tex2Dproj_ps(
	eflib::vec4* results, uint32_t mask,
	salviar::sampler* samp, eflib::vec4* coords,
	eflib::vec4 const* ddxs, eflib::vec4 const* ddys );

END_NS_SALVIAR();

#endif