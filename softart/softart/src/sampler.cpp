#include "../include/sampler.h"

#include "../include/surface.h"
#include "../include/texture.h"
BEGIN_NS_SOFTART()

using namespace efl;
using namespace std;

namespace addresser
{
	float wrap(float coord, int size)
	{
		return (coord - fast_floor(coord)) * size;
	}

	float mirror(float coord, int size)
	{
		float selection_coord = fast_floor(coord);
		return 
			(fast_floori(selection_coord) % 2 == 0 
			? coord - selection_coord
			: 1 + selection_coord - coord) * size;
	}

	float clamp(float coord, int size)
	{
		return efl::clamp(coord * size, 0.5f, size - 0.5f);
	}

	float border(float coord, int size)
	{
		return efl::clamp(coord * size, -0.5f, size + 0.5f);
	}
};

namespace coord_calculator
{
	void nearest_wrap(int& low, int& /*up*/, float& /*frac*/, float coord, int size)
	{
		float o_coord = addresser::wrap(coord, size);
		low = (fast_floori(o_coord) + size) % size;
	}

	void nearest_mirror(int& low, int& /*up*/, float& /*frac*/, float coord, int size)
	{
		float o_coord = addresser::mirror(coord, size);
		low = clamp(fast_floori(o_coord), 0, size - 1);
	}

	void nearest_clamp(int& low, int& /*up*/, float& /*frac*/, float coord, int size)
	{
		float o_coord = addresser::clamp(coord, size);
		int rv = fast_floori(o_coord);
		low = clamp(rv, 0, size - 1);
	}

	void nearest_border(int& low, int& /*up*/, float& /*frac*/, float coord, int size)
	{
		float o_coord = addresser::border(coord, size);
		int rv = fast_floori(o_coord);
		low = rv >= size ? -1 : rv;
	}

	void linear_wrap(int& low, int& up, float& frac, float coord, int size)
	{
		float o_coord = addresser::wrap(coord, size) - 0.5f;
		int coord_ipart = fast_floori(o_coord);

		low = coord_ipart;
		up = low + 1;

		low = (size + low) % size;
		up = (size + up) % size;

		frac = o_coord - coord_ipart;
	}

	void linear_mirror(int& low, int& up, float& frac, float coord, int size)
	{
		float o_coord = addresser::mirror(coord, size) - 0.5f;
		int coord_ipart = fast_floori(o_coord);

		low = coord_ipart;
		up = low + 1;

		low = efl::clamp(low, 0, size - 1);
		up = efl::clamp(up, 0, size - 1);

		frac = o_coord - coord_ipart;
	}

	void linear_clamp(int& low, int& up, float& frac, float coord, int size)
	{
		float o_coord = addresser::clamp(coord, size) - 0.5f;
		int coord_ipart = fast_floori(o_coord);

		low = coord_ipart;
		up = low + 1;

		low = efl::clamp(low, 0, size - 1);
		up = efl::clamp(up, 0, size - 1);

		frac = o_coord - coord_ipart;
	}

	void linear_border(int& low, int& up, float& frac, float coord, int size)
	{
		float o_coord = addresser::border(coord, size) - 0.5f;
		int coord_ipart = fast_floori(o_coord);

		low = coord_ipart;
		up = low + 1;

		low = (low >= size ? -1 : low);
		up = (up >= size ? -1 : up);

		frac = o_coord - coord_ipart;
	}

	const sampler::coord_calculator_op_type
		op_table[address_mode_count][filter_type_count] =
	{
		{ &nearest_wrap, &linear_wrap },
		{ &nearest_mirror, &linear_mirror },
		{ &nearest_clamp, &linear_clamp },
		{ &nearest_border, &linear_border }
	};
};

namespace surface_sampler
{
	color_rgba32f nearest(const surface& surf, float x, float y, const sampler::coord_calculator_op_type* addr_mode0, const sampler::coord_calculator_op_type* addr_mode1, const color_rgba32f& border_color)
	{
		int ix, iy;
		float frac;
		addr_mode0[filter_point](ix, ix, frac, x, int(surf.get_width()));
		addr_mode1[filter_point](iy, iy, frac, y, int(surf.get_height()));

		if(ix < 0 || iy < 0) return border_color;
		return surf.get_texel(ix, iy);
	}

	color_rgba32f linear(const surface& surf, float x, float y, const sampler::coord_calculator_op_type* addr_mode0, const sampler::coord_calculator_op_type* addr_mode1, const color_rgba32f& /*border_color*/)
	{
		int xpos0, ypos0, xpos1, ypos1;
		float tx, ty;

		addr_mode0[filter_linear](xpos0, xpos1, tx, x, int(surf.get_width()));
		addr_mode1[filter_linear](ypos0, ypos1, ty, y, int(surf.get_height()));

		color_rgba32f c0, c1, c2, c3;

		c0 = surf.get_texel(xpos0, ypos0);
		c1 = surf.get_texel(xpos1, ypos0);
		c2 = surf.get_texel(xpos0, ypos1);
		c3 = surf.get_texel(xpos1, ypos1);

		color_rgba32f c01 = lerp(c0, c1, tx);
		color_rgba32f c23 = lerp(c2, c3, tx);

		return lerp(c01, c23, ty);
	}

	const sampler::filter_op_type op_table[filter_type_count] = {&nearest, &linear};
};

float sampler::calc_lod(const vec4& attribute, const vec4& size, const vec4& ddx, const vec4& ddy, float inv_x_w, float inv_y_w, float inv_w, float bias) const
{
	efl::vec4 ddx2 = (attribute + ddx) * inv_x_w - attribute * inv_w;
	efl::vec4 ddy2 = (attribute + ddy) * inv_y_w - attribute * inv_w;

	float rho, lambda;

	vec4 maxD(
		max(abs(ddx2.x), abs(ddy2.x)), 
		max(abs(ddx2.y), abs(ddy2.y)), 
		max(abs(ddx2.z), abs(ddy2.z)), 
		0.0f);
	maxD *= size;

	rho = max(max(maxD.x, maxD.y), maxD.z);
	if(rho == 0.0f) rho = 0.000001f;
	lambda = fast_log2(rho);
	return lambda + bias;
}

color_rgba32f sampler::sample_surface(
	const surface& surf,
	float x, float y,
	sampler_state ss) const
{
	return filters_[ss](
			surf, 
			x, y,
			addr_modes_[sampler_axis_u], addr_modes_[sampler_axis_v],
			desc_.border_color
			);
}

sampler::sampler(const sampler_desc& desc)
	: desc_(desc)
{
	filters_[sampler_state_min] = surface_sampler::op_table[desc.min_filter];
	filters_[sampler_state_mag] = surface_sampler::op_table[desc.mag_filter];
	filters_[sampler_state_mip] = surface_sampler::op_table[desc.mip_filter];

	addr_modes_[sampler_axis_u] = coord_calculator::op_table[desc_.addr_mode_u];
	addr_modes_[sampler_axis_v] = coord_calculator::op_table[desc_.addr_mode_v];
	addr_modes_[sampler_axis_w] = coord_calculator::op_table[desc_.addr_mode_w];
}

color_rgba32f sampler::sample_impl(const texture *tex , float coordx, float coordy, float miplevel) const
{
	bool is_mag = true;

	if(desc_.mip_filter == filter_point) {
		is_mag = (miplevel < 0.5f);
	} else {
		is_mag = (miplevel < 0);
	}

	//放大
	if(is_mag){
		return sample_surface(tex->get_surface(tex->get_max_lod()), coordx, coordy, sampler_state_mag);
	}

	if(desc_.mip_filter == filter_point){
		size_t ml = fast_floori(miplevel + 0.5f);
		ml = clamp(ml, tex->get_max_lod(), tex->get_min_lod());

		return sample_surface(tex->get_surface(ml), coordx, coordy, sampler_state_min);
	}

	if(desc_.mip_filter == filter_linear){
		size_t low = fast_floori(miplevel);
		size_t up = low + 1;

		float frac = miplevel - low;

		low = clamp(low, tex->get_max_lod(), tex->get_min_lod());
		up = clamp(up, tex->get_max_lod(), tex->get_min_lod());

		color_rgba32f c0 = sample_surface(tex->get_surface(low), coordx, coordy, sampler_state_min);
		color_rgba32f c1 = sample_surface(tex->get_surface(up), coordx, coordy, sampler_state_min);
	
		return lerp(c0, c1, frac);
	}

	custom_assert(false, "出现了错误的mip filters参数");
	return desc_.border_color;
}

color_rgba32f sampler::sample_impl(const texture *tex , 
								 float coordx, float coordy,
								 const vec4& ddx, const vec4& ddy,
								 float inv_x_w, float inv_y_w, float inv_w, float lod_bias) const
{
	return sample_2d_impl(tex ,
		vec4(coordx, coordy, 0.0f, 1.0f / inv_w),
		ddx, ddy, inv_x_w, inv_y_w, inv_w, lod_bias
		);
}

color_rgba32f sampler::sample_2d_impl(const texture *tex , 
								 const vec4& coord,
								 const vec4& ddx, const vec4& ddy,
								 float inv_x_w, float inv_y_w, float inv_w, float lod_bias) const
{
	float q = inv_w == 0 ? 1.0f : 1.0f / inv_w;
	vec4 origin_coord(coord * q);
	vec4 size((float)tex->get_width(0), (float)tex->get_height(0), (float)tex->get_depth(0), 0.0f);

	float lod = calc_lod(origin_coord, size, ddx, ddy, inv_x_w, inv_y_w, inv_w, lod_bias);
	return sample_impl(tex , coord.x, coord.y, lod);
}


color_rgba32f sampler::sample(float coordx, float coordy, float miplevel) const
{
	return sample_impl(ptex_, coordx, coordy, miplevel);
}

color_rgba32f sampler::sample(
					 float coordx, float coordy, 
					 const efl::vec4& ddx, const efl::vec4& ddy, 
					 float inv_x_w, float inv_y_w, float inv_w, float lod_bias) const
{
	return sample_impl(ptex_, coordx, coordy, ddx, ddy, inv_x_w, inv_y_w, inv_w, lod_bias);
}


color_rgba32f sampler::sample_2d(
						const efl::vec4& coord,
						const efl::vec4& ddx, const efl::vec4& ddy,
						float inv_x_w, float inv_y_w, float inv_w, float lod_bias) const
{
	return sample_2d_impl(ptex_, coord, ddx, ddy, inv_x_w, inv_y_w, inv_w, lod_bias);
}


color_rgba32f sampler::sample_cube(
	float coordx, float coordy, float coordz,
	float miplevel
	) const
{
	cubemap_faces major_dir;
	float s, t, m;

	float x = coordx;
	float y = coordy;
	float z = coordz;

	float ax = abs(x);
	float ay = abs(y);
	float az = abs(z);

	if(ax > ay && ax > az)
	{
		// x max
		m = ax;
		if(x > 0){
			//+x
			s = 0.5f * (z / m + 1.0f);
			t = 0.5f * (y / m + 1.0f);
			major_dir = cubemap_face_positive_x;
		} else {
			//-x

			s = 0.5f * (-z / m + 1.0f);
			t = 0.5f * (y / m + 1.0f);
			major_dir = cubemap_face_negative_x;
		}
	} else {

		if(ay > ax && ay > az){
			m = ay;
			if(y > 0){
				//+y
				s =0.5f * (x / m + 1.0f);
				t = 0.5f * (z / m + 1.0f);
				major_dir = cubemap_face_positive_y;
			} else {
				s = 0.5f * (x / m + 1.0f);
				t = 0.5f * (-z / m + 1.0f);
				major_dir = cubemap_face_negative_y;
			}
		} else {
			m = az;
			if(z > 0){
				//+z
				s = 0.5f * (-x / m + 1.0f);
				t = 0.5f * (y / m + 1.0f);
				major_dir = cubemap_face_positive_z;
			} else {
				s = 0.5f * (x / m + 1.0f);
				t = 0.5f * (y / m + 1.0f);
				major_dir = cubemap_face_negative_z;
			}
		}
	}

	//暂时先不算ddx ddy
	if(ptex_->get_texture_type() != texture_type_cube)
	{
		custom_assert(false , "texture type not texture_type_cube.");
	}
	const texture_cube* pcube = static_cast<const texture_cube*>(ptex_);
	color_rgba32f rv = sample_impl(&pcube->get_face(major_dir) , s, t, miplevel);
	return rv;
}

color_rgba32f sampler::sample_cube(
	const efl::vec4& coord,
	const efl::vec4& ddx,
	const efl::vec4& ddy,
	float inv_x_w, float inv_y_w, float inv_w, float lod_bias
	) const
{
	float q = inv_w == 0 ? 1.0f : 1.0f / inv_w;
	vec4 origin_coord(coord * q);

	if(ptex_->get_texture_type() != texture_type_cube)
	{
		custom_assert(false , "texture type not texture_type_cube.");
	}
	const texture_cube* pcube = static_cast<const texture_cube*>(ptex_);
	float lod = calc_lod(origin_coord, vec4(float(pcube->get_width()), float(pcube->get_height()), 0.0f, 0.0f), ddx, ddy, inv_x_w, inv_y_w, inv_w, lod_bias);
	//return color_rgba32f(vec4(coord.xyz(), 1.0f));
	//return color_rgba32f(invlod, invlod, invlod, 1.0f);
	return sample_cube(coord.x, coord.y, coord.z, lod);
}
END_NS_SOFTART()
