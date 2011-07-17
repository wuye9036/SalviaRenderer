#include "salviar/include/shaderregs_op.h"
#include "salviar/include/shader.h"
BEGIN_NS_SALVIAR()


using namespace boost;
using namespace eflib;

/*****************************************
 * Triangle Info
 ****************************************/
const eflib::vec4& triangle_info::base_vert() const
{
	EFLIB_ASSERT(pbase_vert, "");
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
	return *pbase_vert;
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
}

const vs_output& triangle_info::ddx() const
{
	EFLIB_ASSERT(pddx, "");
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
	return *pddx;
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
}

const vs_output& triangle_info::ddy() const
{
	EFLIB_ASSERT(pddy, "");
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
	return *pddy;
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
}

void triangle_info::set(const eflib::vec4& base_vert, const vs_output& ddx, const vs_output& ddy)
{
	pddx = &ddx;
	pddy = &ddy;
	pbase_vert = &base_vert;
}

/*****************************************
 *  Get Partial Derivation
 *****************************************/
const eflib::vec4& pixel_shader::get_pos_ddx() const{
	return ptriangleinfo_->ddx().position;
}

const eflib::vec4& pixel_shader::get_pos_ddy() const{
	return ptriangleinfo_->ddy().position;
}

const eflib::vec4& pixel_shader::unproj_ddx(size_t iReg) const{
	return ptriangleinfo_->ddx().attributes[iReg];
}

const eflib::vec4& pixel_shader::unproj_ddy(size_t iReg) const{
	return ptriangleinfo_->ddy().attributes[iReg];
}

const eflib::vec4 pixel_shader::ddx(size_t iReg) const
{
	vec4 attr_org_ddx = unproj_ddx(iReg);

	vec4 attr = ppxin_->attributes[iReg];
	vec4 unproj_attr = attr * ppxin_->position.w;

	unproj_attr += attr_org_ddx;

	float new_pos_w = ppxin_->position.w + ptriangleinfo_->ddx().position.w;
	vec4 new_proj_attr = unproj_attr / new_pos_w;

	return new_proj_attr - attr;
}

const eflib::vec4 pixel_shader::ddy(size_t iReg) const
{
	vec4 attr_org_ddy = unproj_ddy(iReg);

	vec4 attr = ppxin_->attributes[iReg];
	vec4 unproj_attr = attr * ppxin_->position.w;

	unproj_attr += attr_org_ddy;

	float new_pos_w = ppxin_->position.w + ptriangleinfo_->ddy().position.w;
	vec4 new_proj_attr = unproj_attr / new_pos_w;

	return new_proj_attr - attr;
}
/*****************************************
 * Sample Texture
 ****************************************/
color_rgba32f pixel_shader::tex2d(const sampler& s, const vec4& proj_coord, const vec4& ddx, const vec4& ddy, float bias){
	return s.sample_2d(
		proj_coord, ddx, ddy,
		1.0f / (ppxin_->position.w + get_pos_ddx().w),
		1.0f / (ppxin_->position.w + get_pos_ddy().w),
		1.0f / ppxin_->position.w,
		bias
		);
}

color_rgba32f pixel_shader::tex2d(const sampler& s, size_t iReg)
{
	return tex2d(s, ppxin_->attributes[iReg], unproj_ddx(iReg), unproj_ddy(iReg));
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
	const eflib::vec4& attr = ppxin_->attributes[iReg];

	float invq = (attr.w == 0.0f) ? 1.0f : 1.0f / attr.w;

	// If texture is projection,
	// the 'q'('w') component of texture contains the correct perspective information. (not camera projection)

	// It means that, it is unprojected in projection space.
	// So it will be converted to projected space.
	eflib::vec4 ts_proj_attr;
	ts_proj_attr *= invq;
	ts_proj_attr += vec4(1.0f, 1.0f, 0.0f, 0.0f);
	ts_proj_attr *= 0.5f;
	ts_proj_attr.w = attr.w;

	float next_x_inv_w = 1.0 / ( get_pos_ddx().w + ppxin_->position.w );
	float next_y_inv_w = 1.0 / ( get_pos_ddy().w + ppxin_->position.w );

	vec4 v_unproj_attr = attr * ppxin_->position.w;

	vec4 next_x_attr = ( unproj_ddx(iReg) + v_unproj_attr ) * next_x_inv_w;
	vec4 next_y_attr = ( unproj_ddy(iReg) + v_unproj_attr ) * next_y_inv_w;

	return s.sample_2d(
		ts_proj_attr,
		( next_x_attr - attr ) * 0.5f,
		( next_y_attr - attr ) * 0.5f,
		1.0f / next_x_attr.w,
		1.0f / next_y_attr.w,
		invq,
		0.0f
		);
}

color_rgba32f pixel_shader::tex2dproj(const sampler& s, const vec4& v, const vec4& ddx, const vec4& ddy){

	float invq = (v.w == 0.0f) ? 1.0f : 1.0f / v.w;
	//float proj_factor = ppxin_->wpos.w * invq;
	eflib::vec4 projected_v;

	projected_v.xyz() = (v * invq).xyz();
	projected_v  += vec4(1.0f, 1.0f, 0.0f, 0.0f);
	projected_v /= 2.0f;

	projected_v.w = v.w;

	return s.sample_2d(projected_v, ddx, ddy,
		1.0f / (ppxin_->position.w + unproj_ddx(0).w), 1.0f / (ppxin_->position.w + unproj_ddy(0).w), invq, 0.0f);
}

color_rgba32f pixel_shader::texcube(const sampler& s, const eflib::vec4& coord, const eflib::vec4& ddx, const eflib::vec4& ddy, float /*bias*/){
	return s.sample_cube(coord, ddx, ddy,
		1.0f / (ppxin_->position.w + unproj_ddx(0).w), 1.0f / (ppxin_->position.w + unproj_ddy(0).w), 1.0f / ppxin_->position.w, 0.0f);
}

color_rgba32f pixel_shader::texcube(const sampler&s, size_t iReg){
	return texcube(s, ppxin_->attributes[iReg], unproj_ddx(iReg), unproj_ddy(iReg));
}

color_rgba32f pixel_shader::texcubelod(const sampler& s, size_t iReg){
	float x = ppxin_->attributes[iReg].x;
	float y = ppxin_->attributes[iReg].y;
	float z = ppxin_->attributes[iReg].z;

	float lod = ppxin_->attributes[iReg].w;

	return s.sample_cube(x, y, z, lod);
}

color_rgba32f pixel_shader::texcubeproj(const sampler& s, size_t iReg){
	const eflib::vec4& attr = ppxin_->attributes[iReg];

	float invq = (attr.w == 0.0f) ? 1.0f : 1.0f / attr.w;

	//注意这里的透视纹理，不需要除以position.w回到纹理空间，但是由于在光栅化传递进入的时候
	//执行了除法操作，因此需要再乘w回来。
	float proj_factor = ppxin_->position.w * invq;
	eflib::vec4 projected_attr;
	projected_attr.xyz() = (attr * proj_factor).xyz();
	projected_attr.w = attr.w;

	return s.sample_cube(projected_attr, unproj_ddx(iReg), unproj_ddy(iReg),
		1.0f / (ppxin_->position.w + unproj_ddx(0).w), 1.0f / (ppxin_->position.w + unproj_ddy(0).w), invq, 0.0f);
}

color_rgba32f pixel_shader::texcubeproj(const sampler&s, const eflib::vec4& v, const eflib::vec4& ddx, const eflib::vec4& ddy){
	float invq = (v.w == 0.0f) ? 1.0f : 1.0f / v.w;
	float proj_factor = ppxin_->position.w * invq;
	eflib::vec4 projected_v;
	projected_v.xyz() = (v * proj_factor).xyz();
	projected_v.w = v.w;

	return s.sample_cube(projected_v, ddx, ddy,
		1.0f / (ppxin_->position.w + unproj_ddx(0).w), 1.0f / (ppxin_->position.w + unproj_ddy(0).w), invq, 0.0f);
}

/******************************************
 * Execution
 *****************************************/
bool pixel_shader::execute(const vs_output& in, ps_output& out){
	EFLIB_ASSERT(ptriangleinfo_, "");
	ppxin_ = &in;
	bool rv = shader_prog(in, out);
	out.depth = in.position.z;
	out.front_face = in.front_face;
	return rv;
}
END_NS_SALVIAR()
