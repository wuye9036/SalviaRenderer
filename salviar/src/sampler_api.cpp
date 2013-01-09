#include <salviar/include/sampler_api.h>

#include <salviar/include/sampler.h>

#include <eflib/include/utility/unref_declarator.h>

using salviar::sampler;
using eflib::vec2;
using eflib::vec4;

BEGIN_NS_SALVIAR();

void tex2Dgrad_ps(
	vec4* result, uint32_t mask,
	sampler* samp, vec2* coord, vec2 const* ddx, vec2 const* ddy )
{
	if(mask)
	{
		*result = samp->sample_2d_grad(*coord, *ddx, *ddy, 0.0f).get_vec4();
	}
}

void tex2Dbias_ps(
	vec4* results, uint32_t mask,
	sampler* samp, vec4* coords, vec2 const* ddxs, vec2 const* ddys)
{
	EFLIB_UNREF_DECLARATOR(results);
	EFLIB_UNREF_DECLARATOR(mask);
	EFLIB_UNREF_DECLARATOR(samp);
	EFLIB_UNREF_DECLARATOR(coords);
	EFLIB_UNREF_DECLARATOR(ddxs);
	EFLIB_UNREF_DECLARATOR(ddys);

	EFLIB_ASSERT_UNIMPLEMENTED();
}

void tex2Dlod_ps(vec4* results, uint32_t mask, sampler* samp, vec4* coords)
{
	EFLIB_UNREF_DECLARATOR(results);
	EFLIB_UNREF_DECLARATOR(mask);
	EFLIB_UNREF_DECLARATOR(samp);
	EFLIB_UNREF_DECLARATOR(coords);

	EFLIB_ASSERT_UNIMPLEMENTED();
}

void tex2Dproj_ps(vec4* results, uint32_t mask, sampler* samp, vec4* coords, vec4 const* ddxs, vec4 const* ddys )
{
	EFLIB_UNREF_DECLARATOR(results);
	EFLIB_UNREF_DECLARATOR(mask);
	EFLIB_UNREF_DECLARATOR(samp);
	EFLIB_UNREF_DECLARATOR(coords);
	EFLIB_UNREF_DECLARATOR(ddxs);
	EFLIB_UNREF_DECLARATOR(ddys);
	
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void tex2Dlod(vec4& result, sampler* samp, vec4& coord)
{
	result = samp->sample_2d_lod( *(vec2*)(&coord), coord.w() ).get_vec4();
}

void texCUBElod(vec4& result, sampler* samp, vec4& coord)
{
	EFLIB_UNREF_DECLARATOR(result);
	EFLIB_UNREF_DECLARATOR(samp);
	EFLIB_UNREF_DECLARATOR(coord);
	
	EFLIB_ASSERT_UNIMPLEMENTED();
}

END_NS_SALVIAR();