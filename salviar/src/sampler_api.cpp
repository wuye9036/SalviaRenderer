#include <salviar/include/sampler_api.h>

#include <salviar/include/sampler.h>

using eflib::vec2;
using eflib::vec4;

void salviar_tex2Dgrad_pkg( eflib::vec4* results, uint16_t mask, salviar::sampler* samp, eflib::vec2* coords, eflib::vec2 const* ddxs, eflib::vec2 const* ddys )
{
	unsigned long low_start_index;
	while( _BitScanForward( &low_start_index, mask ) ){
		unsigned long index = 15 - low_start_index;
		results[index] = samp->sample_2d_grad(coords[index], ddxs[index], ddys[index], 0.0f).get_vec4();
		mask &= mask - 1;
	}
}

void salviar_tex2Dbias_pkg( eflib::vec4* results, uint16_t mask, salviar::sampler* samp, eflib::vec4* coords, eflib::vec2 const* ddxs, eflib::vec2 const* ddys )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void salviar_tex2Dlod_pkg( eflib::vec4* results, uint16_t mask, salviar::sampler* samp, eflib::vec4* coords )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void salviar_tex2Dproj_pkg( eflib::vec4* results, uint16_t mask, salviar::sampler* samp, eflib::vec4* coords, eflib::vec4 const* ddxs, eflib::vec4 const* ddys )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void salviar_tex2Dlod( eflib::vec4& result, salviar::sampler* samp, eflib::vec4& coord )
{
	result = samp->sample_2d_lod( *(vec2*)(&coord), coord.w() ).get_vec4();
}

