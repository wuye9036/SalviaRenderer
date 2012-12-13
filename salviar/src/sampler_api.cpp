#include <salviar/include/sampler_api.h>

#include <salviar/include/sampler.h>

using salviar::sampler;
using eflib::vec2;
using eflib::vec4;

void salviar_tex2Dgrad_pkg(
	vec4* result, uint32_t mask, sampler* samp, vec2* coord, vec2 const* ddx, vec2 const* ddy
	)
{
	if(mask)
	{
		*result = samp->sample_2d_grad(*coord, *ddx, *ddy, 0.0f).get_vec4();
	}
}

void salviar_tex2Dbias_pkg( eflib::vec4* results, uint32_t mask, salviar::sampler* samp, eflib::vec4* coords, eflib::vec2 const* ddxs, eflib::vec2 const* ddys )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void salviar_tex2Dlod_pkg( eflib::vec4* results, uint32_t mask, salviar::sampler* samp, eflib::vec4* coords )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void salviar_tex2Dproj_pkg( eflib::vec4* results, uint32_t mask, salviar::sampler* samp, eflib::vec4* coords, eflib::vec4 const* ddxs, eflib::vec4 const* ddys )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void salviar_tex2Dlod( eflib::vec4& result, salviar::sampler* samp, eflib::vec4& coord )
{
	result = samp->sample_2d_lod( *(vec2*)(&coord), coord.w() ).get_vec4();
}

void salviar_texCUBElod( eflib::vec4& result, salviar::sampler* samp, eflib::vec4& coord )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}
