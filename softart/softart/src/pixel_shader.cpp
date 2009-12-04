#include "softart/include/shaderregs_op.h"
#include "softart/include/shader.h"

using namespace boost;
using namespace efl;

/*****************************************
 * Triangle Info
 ****************************************/
const efl::vec4& triangle_info::base_vert() const
{
	custom_assert(pbase_vert, "");
	return *pbase_vert;
}

const vs_output& triangle_info::ddx() const
{
	custom_assert(pddx, "");
	return *pddx;
}

const vs_output& triangle_info::ddy() const
{
	custom_assert(pddy, "");
	return *pddy;
}

void triangle_info::set(const efl::vec4& base_vert, const vs_output& ddx, const vs_output& ddy)
{
	pddx = &ddx;
	pddy = &ddy;
	pbase_vert = &base_vert;
}

/*****************************************
 *  Get Partial Derivation
 *****************************************/
const efl::vec4& pixel_shader::get_original_ddx(size_t iReg){
	return ptriangleinfo_->ddx().attributes[iReg];
}

const efl::vec4& pixel_shader::get_original_ddy(size_t iReg){
	return ptriangleinfo_->ddy().attributes[iReg];
}

const efl::vec4 pixel_shader::ddx(size_t iReg)
{
	vec4 attr_org_ddx = get_original_ddx(iReg);

	vec4 attr = ppxin_->attributes[iReg];
	vec4 unproj_attr = attr * ppxin_->wpos.w;

	unproj_attr += attr_org_ddx;

	float new_pos_w = ppxin_->wpos.w + ptriangleinfo_->ddx().wpos.w;
	vec4 new_proj_attr = unproj_attr / new_pos_w;

	return new_proj_attr - attr;
}

const efl::vec4 pixel_shader::ddy(size_t iReg)
{
	vec4 attr_org_ddy = get_original_ddy(iReg);

	vec4 attr = ppxin_->attributes[iReg];
	vec4 unproj_attr = attr * ppxin_->wpos.w;

	unproj_attr += attr_org_ddy;

	float new_pos_w = ppxin_->wpos.w + ptriangleinfo_->ddy().wpos.w;
	vec4 new_proj_attr = unproj_attr / new_pos_w;

	return new_proj_attr - attr;
}
/*****************************************
 * Sample Texture
 ****************************************/
color_rgba32f pixel_shader::tex2d(const sampler& s, const vec4& coord, const vec4& ddx, const vec4& ddy, float /*bias*/){
	return s.sample_2d(coord, ddx, ddy, 1.0f / ppxin_->wpos.w, 0.0f);
}

color_rgba32f pixel_shader::tex2d(const sampler& s, size_t iReg)
{
	return tex2d(s, ppxin_->attributes[iReg], get_original_ddx(iReg), get_original_ddy(iReg));
}

color_rgba32f pixel_shader::tex2dlod(const sampler& s, size_t iReg)
{
	float x = ppxin_->attributes[iReg].x;
	float y = ppxin_->attributes[iReg].y;
	float lod = ppxin_->attributes[iReg].w;

	return s.sample(x, y, lod);
}

color_rgba32f pixel_shader::tex2dproj(const sampler& s, size_t iReg)
{
	const efl::vec4& attr = ppxin_->attributes[iReg];

	float invq = (attr.w == 0.0f) ? 1.0f : 1.0f / attr.w;

	//注意这里的透视纹理，不需要除以position.w回到纹理空间，但是由于在光栅化传递进入的时候
	//执行了除法操作，因此需要再乘w回来。
	//float proj_factor = ppxin_->wpos.w * invq;
	efl::vec4 projected_attr;
	projected_attr.xyz() = (attr * invq).xyz();
	projected_attr  += vec4(1.0f, 1.0f, 0.0f, 0.0f);
	projected_attr /= 2.0f;
	projected_attr.w = attr.w;

	return s.sample_2d(projected_attr, get_original_ddx(iReg), get_original_ddy(iReg), invq, 0.0f);
}

color_rgba32f pixel_shader::tex2dproj(const sampler& s, const vec4& v, const vec4& ddx, const vec4& ddy){

	float invq = (v.w == 0.0f) ? 1.0f : 1.0f / v.w;
	//float proj_factor = ppxin_->wpos.w * invq;
	efl::vec4 projected_v;

	projected_v.xyz() = (v * invq).xyz();
	projected_v  += vec4(1.0f, 1.0f, 0.0f, 0.0f);
	projected_v /= 2.0f;

	projected_v.w = v.w;

	return s.sample_2d(projected_v, ddx, ddy, invq, 0.0f);
}

color_rgba32f pixel_shader::texcube(const sampler& s, const efl::vec4& coord, const efl::vec4& ddx, const efl::vec4& ddy, float /*bias*/){
	return s.sample_cube(coord, ddx, ddy, 1.0f / ppxin_->wpos.w, 0.0f);
}

color_rgba32f pixel_shader::texcube(const sampler&s, size_t iReg){
	return texcube(s, ppxin_->attributes[iReg], get_original_ddx(iReg), get_original_ddy(iReg));
}

color_rgba32f pixel_shader::texcubelod(const sampler& s, size_t iReg){
	float x = ppxin_->attributes[iReg].x;
	float y = ppxin_->attributes[iReg].y;
	float z = ppxin_->attributes[iReg].z;

	float lod = ppxin_->attributes[iReg].w;

	return s.sample_cube(x, y, z, lod);
}

color_rgba32f pixel_shader::texcubeproj(const sampler& s, size_t iReg){
	const efl::vec4& attr = ppxin_->attributes[iReg];

	float invq = (attr.w == 0.0f) ? 1.0f : 1.0f / attr.w;

	//注意这里的透视纹理，不需要除以position.w回到纹理空间，但是由于在光栅化传递进入的时候
	//执行了除法操作，因此需要再乘w回来。
	float proj_factor = ppxin_->wpos.w * invq;
	efl::vec4 projected_attr;
	projected_attr.xyz() = (attr * proj_factor).xyz();
	projected_attr.w = attr.w;

	return s.sample_cube(projected_attr, get_original_ddx(iReg), get_original_ddy(iReg), invq, 0.0f);
}

color_rgba32f pixel_shader::texcubeproj(const sampler&s, const efl::vec4& v, const efl::vec4& ddx, const efl::vec4& ddy){
	float invq = (v.w == 0.0f) ? 1.0f : 1.0f / v.w;
	float proj_factor = ppxin_->wpos.w * invq;
	efl::vec4 projected_v;
	projected_v.xyz() = (v * proj_factor).xyz();
	projected_v.w = v.w;

	return s.sample_cube(projected_v, ddx, ddy, invq, 0.0f);
}

/******************************************
 * Execution
 *****************************************/
bool pixel_shader::execute(const vs_output& in, ps_output& out){
	custom_assert(ptriangleinfo_, "");
	ppxin_ = &in;
	bool rv = shader_prog(in, out);
	out.depth = in.wpos.z;
	return rv;
}