#include "../include/sampler.h"

#include "../include/surface.h"
#include "../include/texture.h"
BEGIN_NS_SOFTART()

using namespace efl;
using namespace std;

namespace addresser
{
	struct wrap
	{
		static float op1(float coord, int size)
		{
			return (coord - fast_floor(coord)) * size - 0.5f;
		}
		static vec2 op1(const vec2& coord, const int2& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_set_ps(0, 0, coord.y, coord.x);
			__m128i micoord = _mm_cvttps_epi32(mfcoord);
			__m128 tmp = _mm_cvtepi32_ps(micoord);
			tmp = _mm_sub_ps(mfcoord, tmp);
			__m128 msize = _mm_set_ps(0, 0, static_cast<float>(size.y), static_cast<float>(size.x));
			tmp = _mm_mul_ps(msize, tmp);
			const __m128 mhalf = _mm_set1_ps(0.5f);
			tmp = _mm_sub_ps(tmp, mhalf);
			float f[4];
			_mm_storeu_ps(f, tmp);
			return vec2(f[0], f[1]);
#else
			return (coord - vec2(fast_floor(coord.x), fast_floor(coord.y))) * vec2(size.x, size.y) - 0.5f;
#endif
		}

		static int op2(int coord, int size)
		{
			return (size * 8192 + coord) % size;
		}
		static int2 op2(int2 coord, int2 size)
		{
#ifndef EFLIB_NO_SIMD
			__m128i micoord = _mm_set_epi32(0, coord.y, 0, coord.x);
			__m128i misize = _mm_set_epi32(0, size.y, 0, size.x);
			micoord = _mm_add_epi32(micoord, _mm_slli_si128(misize, 2));
			__m128 mfcoord = _mm_cvtepi32_ps(micoord);
			__m128 mfsize = _mm_cvtepi32_ps(misize);
			__m128i midiv = _mm_cvttps_epi32(_mm_div_ps(mfcoord, mfsize));
			__m128i mir = _mm_sub_epi32(micoord, _mm_mul_epu32(midiv, misize));
			int i[4];
			_mm_storeu_si128(reinterpret_cast<__m128i*>(i), mir);
			return int2(i[0], i[2]);
#else
			return int2((size.x * 8192 + coord.x) % size.x,
				(size.y * 8192 + coord.y) % size.y);
#endif
		}
	};

	struct mirror
	{
		static float op1(float coord, int size)
		{
			int selection_coord = fast_floori(coord);
			return 
				(selection_coord & 1 
				? 1 + selection_coord - coord
				: coord - selection_coord) * size - 0.5f;
		}
		static vec2 op1(const vec2& coord, const int2& size)
		{
			int selection_coord_x = fast_floori(coord.x);
			int selection_coord_y = fast_floori(coord.y);
			return 
				vec2((selection_coord_x & 1 
				? 1 + selection_coord_x - coord.x
				: coord.x - selection_coord_x) * size.x - 0.5f,
				(selection_coord_y & 1 
				? 1 + selection_coord_y - coord.y
				: coord.y - selection_coord_y) * size.y - 0.5f);
		}

		static int op2(int coord, int size)
		{
			return efl::clamp(coord, 0, size - 1);
		}
		static int2 op2(const int2& coord, const int2& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128i micoord = _mm_set_epi32(0, 0, coord.y, coord.x);
			__m128i misize = _mm_set_epi32(0, 0, size.y, size.x);
			misize = _mm_sub_epi32(misize, _mm_set1_epi32(1));
			__m128i tmp = _mm_max_epi16(micoord, _mm_set1_epi32(0));
			tmp = _mm_min_epi16(tmp, misize);
			int i[4];
			_mm_storeu_si128(reinterpret_cast<__m128i*>(i), tmp);
			return int2(i[0], i[1]);
#else
			return int2(efl::clamp(coord.x, 0, size.x - 1),
				efl::clamp(coord.y, 0, size.y - 1));
#endif
		}
	};

	struct clamp
	{
		static float op1(float coord, int size)
		{
			return efl::clamp(coord * size, 0.5f, size - 0.5f) - 0.5f;
		}
		static vec2 op1(const vec2& coord, const int2& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_set_ps(0, 0, coord.y, coord.x);
			__m128 mfsize = _mm_set_ps(0, 0, static_cast<float>(size.y), static_cast<float>(size.x));
			const __m128 mhalf = _mm_set1_ps(0.5f);
			__m128 tmp = _mm_mul_ps(mfcoord, mfsize);
			tmp = _mm_max_ps(tmp, mhalf);
			tmp = _mm_min_ps(tmp, _mm_sub_ps(mfsize, mhalf));
			tmp = _mm_sub_ps(tmp, mhalf);
			float f[4];
			_mm_storeu_ps(f, tmp);
			return vec2(f[0], f[1]);
#else
			return vec2(efl::clamp(coord.x * size.x, 0.5f, size.x - 0.5f) - 0.5f,
				efl::clamp(coord.y * size.y, 0.5f, size.y - 0.5f) - 0.5f);
#endif
		}

		static int op2(int coord, int size)
		{
			return efl::clamp(coord, 0, size - 1);
		}
		static int2 op2(const int2& coord, const int2& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128i micoord = _mm_set_epi32(0, 0, coord.y, coord.x);
			__m128i misize = _mm_set_epi32(0, 0, size.y, size.x);
			misize = _mm_sub_epi32(misize, _mm_set1_epi32(1));
			__m128i tmp = _mm_max_epi16(micoord, _mm_set1_epi32(0));
			tmp = _mm_min_epi16(tmp, misize);
			int i[4];
			_mm_storeu_si128(reinterpret_cast<__m128i*>(i), tmp);
			return int2(i[0], i[1]);
#else
			return int2(efl::clamp(coord.x, 0, size.x - 1),
				efl::clamp(coord.y, 0, size.y - 1));
#endif
		}
	};

	struct border
	{
		static float op1(float coord, int size)
		{
			return efl::clamp(coord * size, -0.5f, size + 0.5f) - 0.5f;
		}
		static vec2 op1(const vec2& coord, const int2& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_set_ps(0, 0, coord.y, coord.x);
			__m128 mfsize = _mm_set_ps(0, 0, static_cast<float>(size.y), static_cast<float>(size.x));
			const __m128 mneghalf = _mm_set1_ps(-0.5f);
			__m128 tmp = _mm_mul_ps(mfcoord, mfsize);
			tmp = _mm_max_ps(tmp, mneghalf);
			tmp = _mm_min_ps(tmp, _mm_sub_ps(mfsize, mneghalf));
			tmp = _mm_add_ps(tmp, mneghalf);
			float f[4];
			_mm_storeu_ps(f, tmp);
			return vec2(f[0], f[1]);
#else
			return vec2(efl::clamp(coord.x * size.x, -0.5f, size.x + 0.5f) - 0.5f,
				efl::clamp(coord.y * size.y, -0.5f, size.y + 0.5f) - 0.5f);
#endif
		}

		static int op2(int coord, int size)
		{
			return coord >= size ? -1 : coord;
		}
		static int2 op2(const int2& coord, const int2& size)
		{
			return int2(coord.x >= size.x ? -1 : coord.x,
				coord.y >= size.y ? -1 : coord.y);
		}
	};
};

namespace coord_calculator
{
	template <typename addresser_type>
	int nearest_cc(float coord, int size)
	{
		float o_coord = addresser_type::op1(coord, size);
		int coord_ipart = fast_floori(o_coord + 0.5f);
		return addresser_type::op2(coord_ipart, size);
	}

	template <typename addresser_type>
	void linear_cc(int& low, int& up, float& frac, float coord, int size)
	{
		float o_coord = addresser_type::op1(coord, size);
		int coord_ipart = fast_floori(o_coord);
		low = addresser_type::op2(coord_ipart, size);
		up = addresser_type::op2(coord_ipart + 1, size);
		frac = o_coord - coord_ipart;
	}

	template <typename addresser_type>
	int2 nearest_cc(const vec2& coord, const int2& size)
	{
		vec2 o_coord = addresser_type::op1(coord, size);
		int2 coord_ipart = int2(fast_roundi(o_coord.x), fast_roundi(o_coord.y));
		return addresser_type::op2(coord_ipart, size);
	}

	template <typename addresser_type>
	void linear_cc(int2& low, int2& up, vec2& frac, const vec2& coord, const int2& size)
	{
		vec2 o_coord = addresser_type::op1(coord, size);
		int2 coord_ipart = int2(fast_floori(o_coord.x), fast_floori(o_coord.y));
		low = addresser_type::op2(coord_ipart, size);
		up = addresser_type::op2(coord_ipart + 1, size);
		frac = o_coord - vec2(static_cast<float>(coord_ipart.x), static_cast<float>(coord_ipart.y));
	}
};

namespace surface_sampler
{
	template <typename addresser_type_u, typename addresser_type_v>
	struct nearest
	{
		static color_rgba32f op(const surface& surf, float x, float y, const color_rgba32f& border_color)
		{
			int ix = coord_calculator::nearest_cc<addresser_type_u>(x, int(surf.get_width()));
			int iy = coord_calculator::nearest_cc<addresser_type_v>(y, int(surf.get_height()));

			if(ix < 0 || iy < 0) return border_color;
			return surf.get_texel(ix, iy);
		}
	};

	template <typename addresser_type_u, typename addresser_type_v>
	struct linear
	{
		static color_rgba32f op(const surface& surf, float x, float y, const color_rgba32f& /*border_color*/)
		{
			int xpos0, ypos0, xpos1, ypos1;
			float tx, ty;

			coord_calculator::linear_cc<addresser_type_u>(xpos0, xpos1, tx, x, int(surf.get_width()));
			coord_calculator::linear_cc<addresser_type_v>(ypos0, ypos1, ty, y, int(surf.get_height()));

			color_rgba32f c0, c1, c2, c3;

			c0 = surf.get_texel(xpos0, ypos0);
			c1 = surf.get_texel(xpos1, ypos0);
			c2 = surf.get_texel(xpos0, ypos1);
			c3 = surf.get_texel(xpos1, ypos1);

			color_rgba32f c01 = lerp(c0, c1, tx);
			color_rgba32f c23 = lerp(c2, c3, tx);

			return lerp(c01, c23, ty);
		}
	};

	template <typename addresser_type_uv>
	struct nearest<addresser_type_uv, addresser_type_uv>
	{
		static color_rgba32f op(const surface& surf, float x, float y, const color_rgba32f& border_color)
		{
			int2 ixy = coord_calculator::nearest_cc<addresser_type_uv>(vec2(x, y), int2(surf.get_width(), surf.get_height()));

			if(ixy.x < 0 || ixy.y < 0) return border_color;
			return surf.get_texel(ixy.x, ixy.y);
		}
	};

	template <typename addresser_type_uv>
	struct linear<addresser_type_uv, addresser_type_uv>
	{
		static color_rgba32f op(const surface& surf, float x, float y, const color_rgba32f& /*border_color*/)
		{
			int2 pos0, pos1;
			vec2 t;

			coord_calculator::linear_cc<addresser_type_uv>(pos0, pos1, t, vec2(x, y), int2(surf.get_width(), surf.get_height()));

			color_rgba32f c0, c1, c2, c3;

			c0 = surf.get_texel(pos0.x, pos0.y);
			c1 = surf.get_texel(pos1.x, pos0.y);
			c2 = surf.get_texel(pos0.x, pos1.y);
			c3 = surf.get_texel(pos1.x, pos1.y);

			color_rgba32f c01 = lerp(c0, c1, t.x);
			color_rgba32f c23 = lerp(c2, c3, t.x);

			return lerp(c01, c23, t.y);
		}
	};

	const sampler::filter_op_type filter_table[filter_type_count][address_mode_count][address_mode_count] = 
	{
		{
			{
				nearest<addresser::wrap, addresser::wrap>::op,
				nearest<addresser::wrap, addresser::mirror>::op,
				nearest<addresser::wrap, addresser::clamp>::op,
				nearest<addresser::wrap, addresser::border>::op
			},
			{
				nearest<addresser::mirror, addresser::wrap>::op,
				nearest<addresser::mirror, addresser::mirror>::op,
				nearest<addresser::mirror, addresser::clamp>::op,
				nearest<addresser::mirror, addresser::border>::op
			},
			{
				nearest<addresser::clamp, addresser::wrap>::op,
				nearest<addresser::clamp, addresser::mirror>::op,
				nearest<addresser::clamp, addresser::clamp>::op,
				nearest<addresser::clamp, addresser::border>::op
			},
			{
				nearest<addresser::border, addresser::wrap>::op,
				nearest<addresser::border, addresser::mirror>::op,
				nearest<addresser::border, addresser::clamp>::op,
				nearest<addresser::border, addresser::border>::op
			}
		},
		{
			{
				linear<addresser::wrap, addresser::wrap>::op,
				linear<addresser::wrap, addresser::mirror>::op,
				linear<addresser::wrap, addresser::clamp>::op,
				linear<addresser::wrap, addresser::border>::op
			},
			{
				linear<addresser::mirror, addresser::wrap>::op,
				linear<addresser::mirror, addresser::mirror>::op,
				linear<addresser::mirror, addresser::clamp>::op,
				linear<addresser::mirror, addresser::border>::op
			},
			{
				linear<addresser::clamp, addresser::wrap>::op,
				linear<addresser::clamp, addresser::mirror>::op,
				linear<addresser::clamp, addresser::clamp>::op,
				linear<addresser::clamp, addresser::border>::op
			},
			{
				linear<addresser::border, addresser::wrap>::op,
				linear<addresser::border, addresser::mirror>::op,
				linear<addresser::border, addresser::clamp>::op,
				linear<addresser::border, addresser::border>::op
			}
		}
	};
}

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
			desc_.border_color
			);
}

sampler::sampler(const sampler_desc& desc)
	: desc_(desc)
{
	filters_[sampler_state_min] = surface_sampler::filter_table[desc_.min_filter][desc_.addr_mode_u][desc.addr_mode_v];
	filters_[sampler_state_mag] = surface_sampler::filter_table[desc_.mag_filter][desc_.addr_mode_u][desc.addr_mode_v];
	filters_[sampler_state_mip] = surface_sampler::filter_table[desc_.mip_filter][desc_.addr_mode_u][desc.addr_mode_v];
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
