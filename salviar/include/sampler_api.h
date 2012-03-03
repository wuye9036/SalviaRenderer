#ifndef SALVIAR_SAMPLER_API_H
#define SALVIAR_SAMPLER_API_H

#include <salviar/include/salviar_forward.h>

#include <eflib/include/math/vector.h>

BEGIN_NS_SALVIAR();

class sampler;

END_NS_SALVIAR();

extern "C" 
{
	// MIMD
	void salviar_tex2D( eflib::vec4& result, salviar::sampler* samp, eflib::vec2& coord, eflib::vec2 const& ddx, eflib::vec2 const& ddy );
	void salviar_tex2Dgrad(
		eflib::vec4& result,
		salviar::sampler* samp, eflib::vec2 const& coord,
		eflib::vec2 const& ddx, eflib::vec2 const& ddy );
	void salviar_tex2Dbias(
		eflib::vec4& result,
		salviar::sampler* samp, eflib::vec4& coord,
		eflib::vec2& ddx, eflib::vec2& ddy );
	void salviar_tex2Dlod (
		eflib::vec4& result,
		salviar::sampler* samp, eflib::vec4& coord );
	void salviar_tex2Dproj(
		eflib::vec4& result,
		salviar::sampler* samp, eflib::vec4& coord,
		eflib::vec4& ddx, eflib::vec4& ddy );

	// Packaged version
	void salviar_tex2Dgrad_pkg(
		eflib::vec4* results, uint16_t mask,
		salviar::sampler* samp, eflib::vec2* coords,
		eflib::vec2 const* ddxs, eflib::vec2 const* ddys );
	void salviar_tex2Dbias_pkg(
		eflib::vec4* results, uint16_t mask,
		salviar::sampler* samp, eflib::vec4* coords,
		eflib::vec2 const* ddxs, eflib::vec2 const* ddys );
	void salviar_tex2Dlod_pkg (
		eflib::vec4* results, uint16_t mask,
		salviar::sampler* samp, eflib::vec4* coords );
	void salviar_tex2Dproj_pkg(
		eflib::vec4* results, uint16_t mask,
		salviar::sampler* samp, eflib::vec4* coords,
		eflib::vec4 const* ddxs, eflib::vec4 const* ddys );
}

#endif