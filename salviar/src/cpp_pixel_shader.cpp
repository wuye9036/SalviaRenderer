#include <salviar/include/shader_regs.h>
#include <salviar/include/shader_regs_op.h>
#include <salviar/include/shader.h>

BEGIN_NS_SALVIAR();

using namespace boost;
using namespace eflib;

// ------------------------------------------
//  Get Partial Derivation
eflib::vec4 cpp_pixel_shader::ddx(size_t iReg) const
{
	return quad_[1].attribute(iReg) - quad_[0].attribute(iReg);
}

eflib::vec4 cpp_pixel_shader::ddy(size_t iReg) const
{
	return quad_[2].attribute(iReg) - quad_[0].attribute(iReg);
}

// ---------------------------------------
// Sample Texture
color_rgba32f cpp_pixel_shader::tex2d(const sampler& s, size_t iReg)
{
	if( ( lod_flag_ & (1ULL << iReg) ) == 0 )
	{
		lod_[iReg] = s.calc_lod_2d( ddx(iReg).xy(), ddy(iReg).xy() );
		lod_flag_ |= (1ULL << iReg);
	}
	return s.sample_2d_lod(px_->attribute(iReg).xy(), lod_[iReg]);

	// return s.sample_2d_grad( px_->attribute(iReg).xy(), ddx(iReg).xy(), ddy(iReg).xy(), 0.0f );
}

color_rgba32f cpp_pixel_shader::tex2dlod(sampler const& s, eflib::vec4 const& coord_with_lod)
{
    return s.sample(coord_with_lod[0], coord_with_lod[1], coord_with_lod[3]);
}

color_rgba32f cpp_pixel_shader::tex2dlod(const sampler& s, size_t iReg)
{
    return tex2dlod(s, px_->attribute(iReg));
}

color_rgba32f cpp_pixel_shader::tex2dproj(const sampler& s, size_t iReg)
{
	eflib::vec4 const& attr = px_->attribute(iReg);

	eflib::vec4 dadx = ddx(iReg);
	eflib::vec4 dady = ddy(iReg);

	eflib::vec4 next_x_attr = attr + dadx;
	eflib::vec4 next_y_attr = attr + dady;

	eflib::vec4 proj_coord		  = ( ( attr        / attr.w()        ) + vec4(1.0f, 1.0f, 0.0f, 0.0f) ) * 0.5f;
	eflib::vec4 next_x_proj_coord = ( ( next_x_attr / next_x_attr.w() ) + vec4(1.0f, 1.0f, 0.0f, 0.0f) ) * 0.5f;
	eflib::vec4 next_y_proj_coord = ( ( next_y_attr / next_y_attr.w() ) + vec4(1.0f, 1.0f, 0.0f, 0.0f) ) * 0.5f;

	eflib::vec4 dcdx = next_x_proj_coord - proj_coord;
	eflib::vec4 dcdy = next_y_proj_coord - proj_coord;

	return s.sample_2d_grad(proj_coord.xy(), dcdx.xy(), dcdy.xy(), 0.0f);
}

uint64_t cpp_pixel_shader::execute(vs_output const* quad, ps_output* out, float* /*depth*/)
{
	quad_ = quad;
	lod_flag_ = 0;

	uint64_t mask = 0;
	for(int i = 0; i < 4; ++i)
	{
		px_ = quad + i;
		mask |= (shader_prog(*px_, out[i]) ? 0xFFFF : 0x0) << (i * MAX_SAMPLE_COUNT); 	
	}
	
	return mask;
}

bool cpp_pixel_shader::output_depth() const
{
    return false;
}

END_NS_SALVIAR()
