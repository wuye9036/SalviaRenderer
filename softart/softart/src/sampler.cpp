#include "../include/sampler.h"

#include "../include/surface.h"
#include "../include/texture.h"
BEGIN_NS_SOFTART()

using namespace efl;
using namespace std;

namespace addresser
{
	float wrap(float coord, float /*half_pixel_size*/)
	{
		return coord - floor(coord);
	}

	float mirror(float coord, float /*half_pixel_size*/)
	{
		float selection_coord = floor(coord);
		return 
			int(selection_coord) % 2 == 0 
			? coord - selection_coord
			: 1 + selection_coord - coord;
	}

	float clamp(float coord, float half_pixel_size)
	{
		return efl::clamp(coord, half_pixel_size, 1.0f - half_pixel_size);
	}

	float border(float coord, float half_pixel_size)
	{
		return efl::clamp(coord, -half_pixel_size, 1.0f + half_pixel_size);
	}

	typedef float (*op_type)(float coord, float half_pixel_size);
	const op_type op_table[address_mode_count] = {&wrap, &mirror, &clamp, &border};
};

namespace coord_calculator
{
	int nearest_wrap(float coord, int size)
	{
		float o_coord = addresser::wrap(coord, 0.5f / float(size));
		return (int(floor(o_coord * float(size))) + size) % size;
	}

	int nearest_mirror(float coord, int size)
	{
		float o_coord = addresser::mirror(coord, 0.5f / float(size));
		return clamp(int(floor(o_coord * float(size))), 0, size - 1);
	}

	int nearest_clamp(float coord, int size)
	{
		float o_coord = addresser::clamp(coord, 0.5f / float(size));
		int rv = int(floor(o_coord * float(size)));
		return clamp(rv, 0, size-1);
	}

	int nearest_border(float coord, int size)
	{
		float o_coord = addresser::border(coord, 0.5f / float(size));
		int rv = int(floor(o_coord * float(size)));
		return rv >= size ? -1 : rv;
	}

	void linear_wrap(int& low, int& up, float& frac, float coord, int size)
	{
		float o_coord = addresser::wrap(coord, 0.5f / size)  * float(size) - 0.5f;
		float coord_ipart = floor(o_coord);

		low = (size + int(coord_ipart)) % size;
		up = (size + low + 1) % size;
		frac = o_coord - coord_ipart;
	}

	void linear_mirror(int& low, int& up, float& frac, float coord, int size)
	{
		
		float o_coord = addresser::mirror(coord, 0.5f / size) * float(size) - 0.5f;
		float coord_ipart = floor(o_coord);

		low = int(coord_ipart);
		up = low + 1;

		low = efl::clamp(low, 0, size - 1);
		up = efl::clamp(up, 0, size - 1);
		frac = o_coord - coord_ipart;
	}

	void linear_clamp(int& low, int& up, float& frac, float coord, int size)
	{
		float o_coord = addresser::clamp(coord, 0.5f / size)  * float(size) - 0.5f;
		float coord_ipart = floor(o_coord);

		low = int(coord_ipart);
		up = low + 1;

		low = clamp(low, 0, size-1);
		up = clamp(up, 0, size-1);

		frac = o_coord - coord_ipart;
	}

	void linear_border(int& low, int& up, float& frac, float coord, int size)
	{
		float o_coord = addresser::border(coord, 0.5f / size)  * float(size) - 0.5f;
		float coord_ipart = floor(o_coord);

		low = int(coord_ipart);
		up = low + 1;

		low = (low >= size ? -1 : low);
		up = (up >= size ? -1 : up);
		frac = o_coord - coord_ipart;
	}

	typedef int (*op_type_nearest)(float coord, int size);
	const op_type_nearest 
		op_table_nearest[address_mode_count] =
	{&nearest_wrap, &nearest_mirror, &nearest_clamp, &nearest_border};

	typedef void (*op_type_linear)(int& low, int& up, float& frac, float coord, int size);
	const op_type_linear 
		op_table_linear[address_mode_count] =
	{&linear_wrap, &linear_mirror, &linear_clamp, &linear_border};
};

namespace surface_sampler
{
	color_rgba32f nearest(const surface& surf, float x, float y, address_mode addr_mode0, address_mode addr_mode1, const color_rgba32f& border_color)
	{
		int ix = coord_calculator::op_table_nearest[addr_mode0](x, int(surf.get_width()));
		int iy = coord_calculator::op_table_nearest[addr_mode1](y, int(surf.get_height()));

		if(ix < 0 || iy < 0) return border_color;
		return surf.get_texel(ix, iy);
	}

	color_rgba32f linear(const surface& surf, float x, float y, address_mode addr_mode0, address_mode addr_mode1, const color_rgba32f& /*border_color*/)
	{
		int xpos0, ypos0, xpos1, ypos1;
		float tx, ty;

		coord_calculator::op_table_linear[addr_mode0](xpos0, xpos1, tx, x, int(surf.get_width()));
		coord_calculator::op_table_linear[addr_mode1](ypos0, ypos1, ty, y, int(surf.get_height()));

		color_rgba32f c0, c1, c2, c3;

		c0 = surf.get_texel(xpos0, ypos0);
		c1 = surf.get_texel(xpos1, ypos0);
		c2 = surf.get_texel(xpos0, ypos1);
		c3 = surf.get_texel(xpos1, ypos1);

		color_rgba32f c01 = lerp(c0, 1.0f - tx, c1, tx);
		color_rgba32f c23 = lerp(c2, 1.0f - tx, c3, tx);

		return lerp(c01, 1.0f - ty, c23, ty);
	}

	typedef color_rgba32f (*op_type)(const surface& surf, float x, float y, address_mode addr_mode0, address_mode addr_mode1, const color_rgba32f& border_color);
	const op_type op_table[filter_type_count] = {NULL, &nearest, &linear};
};

float sampler::calc_lod(const vec4& attribute, const vec4& size, const vec4& ddx, const vec4& ddy, float invQ, float bias) const
{
	float wx = attribute.w + ddx.w;
	float wy = attribute.w + ddy.w;
	if(wx == 0.0f) wx = 1.0f;
	if(wy == 0.0f) wy = 1.0f;
	efl::vec4 ddx2 = (attribute + ddx) / wx - attribute * invQ;
	efl::vec4 ddy2 = (attribute + ddy) / wy - attribute * invQ;

	float rho, lambda;

	vec4 maxD(
		max(abs(ddx2.x), abs(ddy2.x)), 
		max(abs(ddx2.y), abs(ddy2.y)), 
		max(abs(ddx2.z), abs(ddy2.z)), 
		0.0f);
	maxD *= size;

	rho = max(	max(maxD.x, maxD.y), maxD.z);
	if(rho == 0.0f) rho = 0.000001f;
	lambda = log2(rho);
	return lambda + bias;
}

color_rgba32f sampler::sample_surface(
	const surface& surf,
	float x, float y,
	sampler_state ss) const
{
	return surface_sampler::op_table[filters_[ss]](
			surf, 
			x, y,
			addr_modes_[sampler_axis_u], addr_modes_[sampler_axis_v],
			border_color_
			);
}

sampler::sampler()
{
	filters_[sampler_state_min] = filters_[sampler_state_mag] = filter_point;
	filters_[sampler_state_mip] = filter_none;

	for(int i = 0; i < sampler_axis_count; ++i){addr_modes_[i] = address_clamp;}
	border_color_ = color_rgba32f(0.0f, 0.0f, 0.0f, 0.0f);
	lod_bias_ = 0;
	max_miplevel_ = 0;
}

void sampler::set_address_mode(sampler_axis axis, address_mode addr_mode)
{
	custom_assert((axis < sampler_axis_count && axis >= 0), "");
	custom_assert((addr_mode < address_mode_count && addr_mode >= 0), "");

	addr_modes_[axis] = addr_mode;
}	

void sampler::set_filter_type(sampler_state sstate, filter_type filter)
{
	custom_assert((0 <= sstate && sstate < sampler_state_count), "");
	custom_assert((0 <= filter && filter <= filter_type_count), "");
	custom_assert( ! (sstate != sampler_state_mip && filter == filter_none), "为非mip采样设置过滤器none!" );

	filters_[sstate] = filter;
}

void sampler::set_lod_bias(float bias)
{
	lod_bias_ = bias;
}

void sampler::set_max_miplevel(int level)
{
	custom_assert(level >= 0, "");
	max_miplevel_ = level;
}

void sampler::set_border_color(const color_rgba32f& border_color)
{
	border_color_ = border_color;
}

color_rgba32f sampler::sample_impl(const texture *tex , float coordx, float coordy, float miplevel) const
{
	bool is_mag = true;

	if(filters_[sampler_state_mip] == filter_point) {
		is_mag = (miplevel < 0.5f);
	} else {
		is_mag = (miplevel < 0);
	}

	//放大
	if(is_mag){
		return sample_surface(	tex->get_surface(tex->get_max_lod()), coordx, coordy,	sampler_state_mag);
	}

	//无Mipmap的缩小采样
	if(filters_[sampler_state_mip] == filter_none){
		return sample_surface(	tex->get_surface(tex->get_max_lod()), coordx, coordy,	sampler_state_min);
	}

	if(filters_[sampler_state_mip] == filter_point){
		size_t ml = size_t(ceil(miplevel + 0.5f)) - 1;
		ml = clamp(ml, tex->get_max_lod(), tex->get_min_lod());

		return sample_surface(	tex->get_surface(ml), coordx, coordy,	sampler_state_min);
	}

	if(filters_[sampler_state_mip] == filter_linear){
		size_t low = (size_t)floor(miplevel);
		size_t up = low + 1;

		low = clamp(low, tex->get_max_lod(), tex->get_min_lod());
		up = clamp(up, tex->get_max_lod(), tex->get_min_lod());
		float frac = miplevel - floor(miplevel);

		color_rgba32f c0 = sample_surface(tex->get_surface(low), coordx, coordy, sampler_state_min);
		color_rgba32f c1 = sample_surface(tex->get_surface(up), coordx, coordy, sampler_state_min);
	
		return lerp(c0, 1.0f - frac, c1, frac);
	}

	custom_assert(false, "出现了错误的mip filters参数");
	return border_color_;
}

color_rgba32f sampler::sample_impl(const texture *tex , 
								 float coordx, float coordy,
								 const vec4& ddx, const vec4& ddy,
								 float invQ, float lod_bias) const
{
	return sample_2d_impl(tex ,
		vec4(coordx, coordy, 0.0f, 1.0f / invQ),
		ddx, ddy, invQ, lod_bias
		);
}

color_rgba32f sampler::sample_2d_impl(const texture *tex , 
								 const vec4& coord,
								 const vec4& ddx, const vec4& ddy,
								 float invQ, float lod_bias) const
{
	float q = invQ == 0 ? 1.0f : 1.0f / invQ;
	vec4 origin_coord(coord * q);
	vec4 size((float)tex->get_width(0), (float)tex->get_height(0), (float)tex->get_depth(0), 0.0f);

	float lod = calc_lod(origin_coord, size, ddx, ddy, invQ, lod_bias);
	return sample_impl(tex , coord.x, coord.y, lod);
}


color_rgba32f sampler::sample(float coordx, float coordy, float miplevel) const
{
	return sample_impl(ptex_, coordx, coordy, miplevel);
}

color_rgba32f sampler::sample(
					 float coordx, float coordy, 
					 const efl::vec4& ddx, const efl::vec4& ddy, 
					 float invQ, float lod_bias) const
{
	return sample_impl(ptex_, coordx, coordy, ddx, ddy, invQ, lod_bias);
}


color_rgba32f sampler::sample_2d(
						const efl::vec4& coord,
						const efl::vec4& ddx, const efl::vec4& ddy,
						float invQ, float lod_bias) const
{
	return sample_2d_impl(ptex_, coord, ddx, ddy, invQ, lod_bias);
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
	float invQ, float lod_bias
	) const
{
	float q = invQ == 0 ? 1.0f : 1.0f / invQ;
	vec4 origin_coord(coord * q);

	if(ptex_->get_texture_type() != texture_type_cube)
	{
		custom_assert(false , "texture type not texture_type_cube.");
	}
	const texture_cube* pcube = static_cast<const texture_cube*>(ptex_);
	float lod = calc_lod(origin_coord, vec4(float(pcube->get_width()), float(pcube->get_height()), 0.0f, 0.0f), ddx, ddy, invQ, lod_bias);
	//return color_rgba32f(vec4(coord.xyz(), 1.0f));
	//return color_rgba32f(invlod, invlod, invlod, 1.0f);
	return sample_cube(coord.x, coord.y, coord.z, lod);
}
END_NS_SOFTART()
