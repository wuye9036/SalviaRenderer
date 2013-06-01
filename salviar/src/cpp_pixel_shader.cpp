#include <salviar/include/shaderregs.h>
#include <salviar/include/shaderregs_op.h>
#include <salviar/include/shader.h>

BEGIN_NS_SALVIAR();

using namespace boost;
using namespace eflib;

const eflib::vec4& triangle_info::base_vert() const
{
	assert(pbase_vert);
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
	assert(pddx);
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
	assert(pddy);
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

// ------------------------------------------
//  Get Partial Derivation
const eflib::vec4& cpp_pixel_shader::get_pos_ddx() const{
	return ptriangleinfo_->ddx().position();
}

const eflib::vec4& cpp_pixel_shader::get_pos_ddy() const{
	return ptriangleinfo_->ddy().position();
}

const eflib::vec4& cpp_pixel_shader::unproj_ddx(size_t iReg) const{
	return ptriangleinfo_->ddx().attribute(iReg);
}

const eflib::vec4& cpp_pixel_shader::unproj_ddy(size_t iReg) const{
	return ptriangleinfo_->ddy().attribute(iReg);
}

const eflib::vec4 cpp_pixel_shader::ddx(size_t iReg) const
{
	vec4 attr_org_ddx = unproj_ddx(iReg);

	vec4 attr = ppxin_->attribute(iReg);
	vec4 unproj_attr = attr * ppxin_->position().w();

	unproj_attr += attr_org_ddx;

	float new_pos_w = ppxin_->position().w() + ptriangleinfo_->ddx().position().w();
	vec4 new_proj_attr = unproj_attr / new_pos_w;

	return new_proj_attr - attr;
}

const eflib::vec4 cpp_pixel_shader::ddy(size_t iReg) const
{
	vec4 attr_org_ddy = unproj_ddy(iReg);

	vec4 attr = ppxin_->attribute(iReg);
	vec4 unproj_attr = attr * ppxin_->position().w();

	unproj_attr += attr_org_ddy;

	float new_pos_w = ppxin_->position().w() + ptriangleinfo_->ddy().position().w();
	vec4 new_proj_attr = unproj_attr / new_pos_w;

	return new_proj_attr - attr;
}

// ---------------------------------------
// Sample Texture

color_rgba32f cpp_pixel_shader::tex2d(const sampler& s, const vec4& proj_coord, const vec4& ddx, const vec4& ddy, float bias){
	return s.sample_2d(
		proj_coord, ddx, ddy,
		1.0f / ( ppxin_->position().w() + get_pos_ddx().w() ),
		1.0f / ( ppxin_->position().w() + get_pos_ddy().w() ),
		1.0f / ppxin_->position().w(),
		bias
		);
}

color_rgba32f cpp_pixel_shader::tex2d(const sampler& s, size_t iReg)
{
	return tex2d(s, ppxin_->attribute(iReg), unproj_ddx(iReg), unproj_ddy(iReg));
}

color_rgba32f cpp_pixel_shader::tex2dlod(const sampler& s, size_t iReg)
{
	float x = ppxin_->attribute(iReg)[0];
	float y = ppxin_->attribute(iReg)[1];
	float lod = ppxin_->attribute(iReg)[3];

	return s.sample(x, y, lod);
}

color_rgba32f cpp_pixel_shader::tex2dproj(const sampler& s, size_t iReg)
{
	const eflib::vec4& attr = ppxin_->attribute(iReg);

	float invq = (attr[3] == 0.0f) ? 1.0f : 1.0f / attr[3];

	// If texture is projection,
	// the 'q'('w') component of texture contains the correct perspective information. (not camera projection)

	// It means that, it is unprojected in projection space.
	// So it will be converted to projected space.
	eflib::vec4 ts_proj_attr;
	ts_proj_attr *= invq;
	ts_proj_attr += vec4(1.0f, 1.0f, 0.0f, 0.0f);
	ts_proj_attr *= 0.5f;
	ts_proj_attr.w( attr[3] );

	float next_x_inv_w = 1.0f / ( get_pos_ddx()[3] + ppxin_->position()[3] );
	float next_y_inv_w = 1.0f / ( get_pos_ddy()[3] + ppxin_->position()[3] );

	vec4 v_unproj_attr = attr * ppxin_->position()[3];

	vec4 next_x_attr = ( unproj_ddx(iReg) + v_unproj_attr ) * next_x_inv_w;
	vec4 next_y_attr = ( unproj_ddy(iReg) + v_unproj_attr ) * next_y_inv_w;

	return s.sample_2d(
		ts_proj_attr,
		( next_x_attr - attr ) * 0.5f,
		( next_y_attr - attr ) * 0.5f,
		1.0f / next_x_attr[3],
		1.0f / next_y_attr[3],
		invq,
		0.0f
		);
}

color_rgba32f cpp_pixel_shader::tex2dproj(const sampler& s, const vec4& v, const vec4& ddx, const vec4& ddy){

	float invq = (v[3] == 0.0f) ? 1.0f : 1.0f / v[3];
	//float proj_factor = ppxin_->wpos[3] * invq;
	eflib::vec4 projected_v;

	projected_v.xyz( (v * invq).xyz() );
	projected_v  += vec4(1.0f, 1.0f, 0.0f, 0.0f);
	projected_v /= 2.0f;

	projected_v.w( v[3] );

	return s.sample_2d(projected_v, ddx, ddy,
		1.0f / (ppxin_->position().w() + unproj_ddx(0)[3] ), 1.0f / (ppxin_->position().w() + unproj_ddy(0)[3] ), invq, 0.0f);
}

color_rgba32f cpp_pixel_shader::texcube(const sampler& s, const eflib::vec4& coord, const eflib::vec4& ddx, const eflib::vec4& ddy, float /*bias*/){
	return s.sample_cube(
		coord, ddx, ddy,
		1.0f / (ppxin_->position().w() + unproj_ddx(0)[3]), 1.0f / (ppxin_->position().w() + unproj_ddy(0)[3] ),
		1.0f / ppxin_->position().w(), 0.0f);
}

color_rgba32f cpp_pixel_shader::texcube(const sampler&s, size_t iReg){
	return texcube(s, ppxin_->attribute(iReg), unproj_ddx(iReg), unproj_ddy(iReg));
}

color_rgba32f cpp_pixel_shader::texcubelod(const sampler& s, size_t iReg){
	float x = ppxin_->attribute(iReg)[0];
	float y = ppxin_->attribute(iReg)[1];
	float z = ppxin_->attribute(iReg)[2];

	float lod = ppxin_->attribute(iReg)[3];

	return s.sample_cube(x, y, z, lod);
}

color_rgba32f cpp_pixel_shader::texcubeproj(const sampler& s, size_t iReg){
	const eflib::vec4& attr = ppxin_->attribute(iReg);

	float invq = (attr[3] == 0.0f) ? 1.0f : 1.0f / attr[3];

	// NOTE: 
	//  Projective texture is special.
	//  We need to divide components by position().w to transform texture
	//  coordinate back to linear texture coordinate space, 
	//  But they had been devided by 'w' when it is passed by rasterizer.
	//  So we need to multiply 'w' to recover the texture coord.
	float proj_factor = ppxin_->position().w() * invq;
	eflib::vec4 projected_attr;
	projected_attr.xyz() = (attr * proj_factor).xyz();
	projected_attr[3] = attr[3];

	return s.sample_cube(projected_attr, unproj_ddx(iReg), unproj_ddy(iReg),
		1.0f / (ppxin_->position().w() + unproj_ddx(0)[3]), 1.0f / (ppxin_->position().w() + unproj_ddy(0)[3]), invq, 0.0f);
}

color_rgba32f cpp_pixel_shader::texcubeproj(const sampler&s, const eflib::vec4& v, const eflib::vec4& ddx, const eflib::vec4& ddy){
	float invq = (v[3] == 0.0f) ? 1.0f : 1.0f / v[3];
	float proj_factor = ppxin_->position().w() * invq;
	eflib::vec4 projected_v;
	projected_v.xyz() = (v * proj_factor).xyz();
	projected_v[3] = v[3];

	return s.sample_cube(projected_v, ddx, ddy,
		1.0f / (ppxin_->position().w() + unproj_ddx(0)[3]), 1.0f / (ppxin_->position().w() + unproj_ddy(0)[3]), invq, 0.0f);
}

bool cpp_pixel_shader::execute(const vs_output& in, ps_output& out)
{
	assert(ptriangleinfo_);
	ppxin_ = &in;
	bool rv = shader_prog(in, out);
	out.depth = in.position().z();
	out.front_face = in.front_face();
	return rv;
}

END_NS_SALVIAR()
