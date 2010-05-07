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
		static float do_coordf(float coord, int size)
		{
			return (coord - fast_floor(coord)) * size - 0.5f;
		}
		static vec4 do_coordf(const vec4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_loadu_ps(&coord.x);
			__m128i micoord = _mm_cvttps_epi32(mfcoord);
			__m128 tmp = _mm_cvtepi32_ps(micoord);
			tmp = _mm_sub_ps(mfcoord, tmp);
			__m128 msize = _mm_cvtepi32_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&size.x)));
			tmp = _mm_mul_ps(msize, tmp);
			const __m128 mhalf = _mm_set1_ps(0.5f);
			tmp = _mm_sub_ps(tmp, mhalf);
			vec4 ret;
			_mm_storeu_ps(&ret.x, tmp);
			return ret;
#else
			return (coord - vec4(fast_floor(coord.x), fast_floor(coord.y), fast_floor(coord.z), fast_floor(coord.w))) * vec4(size.x, size.y, size.z, size.w) - 0.5f;
#endif
		}

		static int do_coordi_point_1d(int coord, int size)
		{
			return (size * 8192 + coord) % size;
		}
		static int4 do_coordi_point_2d(const int4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128i micoord = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&coord.x));
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size.x));
			micoord = _mm_add_epi32(micoord, _mm_slli_si128(misize, 2));
			__m128 mfcoord = _mm_cvtepi32_ps(micoord);
			__m128 mfsize = _mm_cvtepi32_ps(misize);
			__m128i midiv = _mm_cvttps_epi32(_mm_div_ps(mfcoord, mfsize));
			__m128 mfdiv = _mm_cvtepi32_ps(midiv);
			__m128i tmp = _mm_sub_epi32(micoord, _mm_cvttps_epi32(_mm_mul_ps(mfdiv, mfsize)));
			int4 ret;
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&ret.x), tmp);
			return ret;
#else
			return int4((size.x * 8192 + coord.x) % size.x,
				(size.y * 8192 + coord.y) % size.y,
				0, 0);
#endif
		}

		static void do_coordi_linear_2d(int4& low, int4& up, const int4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128i micoord0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&coord.x));
			__m128i micoord01 = _mm_shuffle_epi32(micoord0, _MM_SHUFFLE(1, 0, 1, 0));
			micoord01 = _mm_add_epi32(micoord01, _mm_set_epi32(1, 1, 0, 0));
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size.x));
			misize = _mm_shuffle_epi32(misize, _MM_SHUFFLE(1, 0, 1, 0));
			micoord01 = _mm_add_epi32(micoord01, _mm_slli_si128(misize, 2));
			__m128 mfsize = _mm_cvtepi32_ps(misize);
			__m128 mfcoord = _mm_cvtepi32_ps(micoord01);
			__m128i midiv = _mm_cvttps_epi32(_mm_div_ps(mfcoord, mfsize));
			__m128 mfdiv = _mm_cvtepi32_ps(midiv);
			__m128i tmp = _mm_sub_epi32(micoord01, _mm_cvttps_epi32(_mm_mul_ps(mfdiv, mfsize)));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&low.x), tmp);
			tmp = _mm_shuffle_epi32(tmp, _MM_SHUFFLE(3, 2, 3, 2));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&up.x), tmp);
#else
			low = int4((size.x * 8192 + coord.x) % size.x,
				(size.y * 8192 + coord.y) % size.y,
				0, 0);
			up = int4((size.x * 8192 + coord.x + 1) % size.x,
				(size.y * 8192 + coord.y + 1) % size.y,
				0, 0);
#endif
		}
	};

	struct mirror
	{
		static float do_coordf(float coord, int size)
		{
			int selection_coord = fast_floori(coord);
			return 
				(selection_coord & 1 
				? 1 + selection_coord - coord
				: coord - selection_coord) * size - 0.5f;
		}
		static vec4 do_coordf(const vec4& coord, const int4& size)
		{
			int selection_coord_x = fast_floori(coord.x);
			int selection_coord_y = fast_floori(coord.y);
			return 
				vec4((selection_coord_x & 1 
				? 1 + selection_coord_x - coord.x
				: coord.x - selection_coord_x) * size.x - 0.5f,
				(selection_coord_y & 1 
				? 1 + selection_coord_y - coord.y
				: coord.y - selection_coord_y) * size.y - 0.5f,
				0, 0);
		}

		static int do_coordi_point_1d(int coord, int size)
		{
			return efl::clamp(coord, 0, size - 1);
		}
		static int4 do_coordi_point_2d(const int4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128i micoord = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&coord.x));
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size.x));
			misize = _mm_sub_epi32(misize, _mm_set1_epi32(1));
			__m128i tmp = _mm_max_epi16(micoord, _mm_set1_epi32(0));
			tmp = _mm_min_epi16(tmp, misize);
			int4 ret;
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&ret.x), tmp);
			return ret;
#else
			return int4(efl::clamp(coord.x, 0, size.x - 1),
				efl::clamp(coord.y, 0, size.y - 1),
				0, 0);
#endif
		}

		static void do_coordi_linear_2d(int4& low, int4& up, const int4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128i micoord0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&coord.x));
			__m128i micoord01 = _mm_shuffle_epi32(micoord0, _MM_SHUFFLE(1, 0, 1, 0));
			micoord01 = _mm_add_epi32(micoord01, _mm_set_epi32(1, 1, 0, 0));
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size));
			misize = _mm_shuffle_epi32(misize, _MM_SHUFFLE(1, 0, 1, 0));
			misize = _mm_sub_epi32(misize, _mm_set1_epi32(1));
			__m128i tmp = _mm_max_epi16(micoord01, _mm_set1_epi32(0));
			tmp = _mm_min_epi16(tmp, misize);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&low.x), tmp);
			tmp = _mm_shuffle_epi32(tmp, _MM_SHUFFLE(3, 2, 3, 2));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&up.x), tmp);
#else
			low = int4(efl::clamp(coord.x, 0, size.x - 1),
				efl::clamp(coord.y, 0, size.y - 1),
				0, 0);
			up = int4(efl::clamp(coord.x + 1, 0, size.x - 1),
				efl::clamp(coord.y + 1, 0, size.y - 1),
				0, 0);
#endif
		}
	};

	struct clamp
	{
		static float do_coordf(float coord, int size)
		{
			return efl::clamp(coord * size, 0.5f, size - 0.5f) - 0.5f;
		}
		static vec4 do_coordf(const vec4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_loadu_ps(&coord.x);
			__m128 mfsize = _mm_cvtepi32_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&size.x)));
			const __m128 mhalf = _mm_set1_ps(0.5f);
			__m128 tmp = _mm_mul_ps(mfcoord, mfsize);
			tmp = _mm_max_ps(tmp, mhalf);
			tmp = _mm_min_ps(tmp, _mm_sub_ps(mfsize, mhalf));
			tmp = _mm_sub_ps(tmp, mhalf);
			vec4 ret;
			_mm_storeu_ps(&ret.x, tmp);
			return ret;
#else
			return vec4(efl::clamp(coord.x * size.x, 0.5f, size.x - 0.5f) - 0.5f,
				efl::clamp(coord.y * size.y, 0.5f, size.y - 0.5f) - 0.5f,
				0, 0);
#endif
		}

		static int do_coordi_point_1d(int coord, int size)
		{
			return efl::clamp(coord, 0, size - 1);
		}
		static int4 do_coordi_point_2d(const int4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128i micoord = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&coord.x));
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size));
			misize = _mm_sub_epi32(misize, _mm_set1_epi32(1));
			__m128i tmp = _mm_max_epi16(micoord, _mm_set1_epi32(0));
			tmp = _mm_min_epi16(tmp, misize);
			int4 ret;
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&ret.x), tmp);
			return ret;
#else
			return int4(efl::clamp(coord.x, 0, size.x - 1),
				efl::clamp(coord.y, 0, size.y - 1),
				0, 0);
#endif
		}

		static void do_coordi_linear_2d(int4& low, int4& up, const int4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128i micoord0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&coord.x));
			__m128i micoord01 = _mm_shuffle_epi32(micoord0, _MM_SHUFFLE(1, 0, 1, 0));
			micoord01 = _mm_add_epi32(micoord01, _mm_set_epi32(1, 1, 0, 0));
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size));
			misize = _mm_shuffle_epi32(misize, _MM_SHUFFLE(1, 0, 1, 0));
			misize = _mm_sub_epi32(misize, _mm_set1_epi32(1));
			__m128i tmp = _mm_max_epi16(micoord01, _mm_set1_epi32(0));
			tmp = _mm_min_epi16(tmp, misize);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&low.x), tmp);
			tmp = _mm_shuffle_epi32(tmp, _MM_SHUFFLE(3, 2, 3, 2));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&up.x), tmp);
#else
			low = int4(efl::clamp(coord.x, 0, size.x - 1),
				efl::clamp(coord.y, 0, size.y - 1),
				0, 0);
			up = int4(efl::clamp(coord.x + 1, 0, size.x - 1),
				efl::clamp(coord.y + 1, 0, size.y - 1),
				0, 0);
#endif
		}
	};

	struct border
	{
		static float do_coordf(float coord, int size)
		{
			return efl::clamp(coord * size, -0.5f, size + 0.5f) - 0.5f;
		}
		static vec4 do_coordf(const vec4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_loadu_ps(&coord.x);
			__m128 mfsize = _mm_cvtepi32_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&size.x)));
			const __m128 mneghalf = _mm_set1_ps(-0.5f);
			__m128 tmp = _mm_mul_ps(mfcoord, mfsize);
			tmp = _mm_max_ps(tmp, mneghalf);
			tmp = _mm_min_ps(tmp, _mm_sub_ps(mfsize, mneghalf));
			tmp = _mm_add_ps(tmp, mneghalf);
			vec4 ret;
			_mm_storeu_ps(&ret.x, tmp);
			return ret;
#else
			return vec4(efl::clamp(coord.x * size.x, -0.5f, size.x + 0.5f) - 0.5f,
				efl::clamp(coord.y * size.y, -0.5f, size.y + 0.5f) - 0.5f,
				0, 0);
#endif
		}

		static int do_coordi_point_1d(int coord, int size)
		{
			return coord >= size ? -1 : coord;
		}
		static int4 do_coordi_point_2d(const int4& coord, const int4& size)
		{
			return int4(coord.x >= size.x ? -1 : coord.x,
				coord.y >= size.y ? -1 : coord.y,
				0, 0);
		}

		static void do_coordi_linear_2d(int4& low, int4& up, const int4& coord, const int4& size)
		{
			low = int4(coord.x >= size.x ? -1 : coord.x,
				coord.y >= size.y ? -1 : coord.y,
				0, 0);
			up = int4(coord.x + 1 >= size.x ? -1 : coord.x + 1,
				coord.y + 1 >= size.y ? -1 : coord.y + 1,
				0, 0);
		}
	};
};

namespace coord_calculator
{
	template <typename addresser_type>
	int point_cc(float coord, int size)
	{
		float o_coord = addresser_type::do_coordf(coord, size);
		int coord_ipart = fast_floori(o_coord + 0.5f);
		return addresser_type::do_coordi_point_1d(coord_ipart, size);
	}

	template <typename addresser_type>
	void linear_cc(int& low, int& up, float& frac, float coord, int size)
	{
		float o_coord = addresser_type::do_coordf(coord, size);
		int coord_ipart = fast_floori(o_coord);
		low = addresser_type::do_coordi_point_1d(coord_ipart, size);
		up = addresser_type::do_coordi_point_1d(coord_ipart + 1, size);
		frac = o_coord - coord_ipart;
	}

	template <typename addresser_type>
	int4 point_cc(const vec4& coord, const int4& size)
	{
		vec4 o_coord = addresser_type::do_coordf(coord, size);
		int4 coord_ipart = int4(fast_roundi(o_coord.x), fast_roundi(o_coord.y), 0, 0);
		return addresser_type::do_coordi_point_2d(coord_ipart, size);
	}

	template <typename addresser_type>
	void linear_cc(int4& low, int4& up, vec4& frac, const vec4& coord, const int4& size)
	{
		vec4 o_coord = addresser_type::do_coordf(coord, size);
		int4 coord_ipart = int4(fast_floori(o_coord.x), fast_floori(o_coord.y), 0, 0);
		addresser_type::do_coordi_linear_2d(low, up, coord_ipart, size);
		frac = o_coord - vec4(static_cast<float>(coord_ipart.x), static_cast<float>(coord_ipart.y), 0, 0);
	}
};

namespace surface_sampler
{
	template <typename addresser_type_u, typename addresser_type_v>
	struct point
	{
		static color_rgba32f op(const surface& surf, float x, float y, const color_rgba32f& border_color)
		{
			int ix = coord_calculator::point_cc<addresser_type_u>(x, int(surf.get_width()));
			int iy = coord_calculator::point_cc<addresser_type_v>(y, int(surf.get_height()));

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
	struct point<addresser_type_uv, addresser_type_uv>
	{
		static color_rgba32f op(const surface& surf, float x, float y, const color_rgba32f& border_color)
		{
			int4 ixy = coord_calculator::point_cc<addresser_type_uv>(vec4(x, y, 0, 0),
				int4(static_cast<int>(surf.get_width()), static_cast<int>(surf.get_height()), 0, 0));

			if(ixy.x < 0 || ixy.y < 0) return border_color;
			return surf.get_texel(ixy.x, ixy.y);
		}
	};

	template <typename addresser_type_uv>
	struct linear<addresser_type_uv, addresser_type_uv>
	{
		static color_rgba32f op(const surface& surf, float x, float y, const color_rgba32f& /*border_color*/)
		{
			int4 pos0, pos1;
			vec4 t;

			coord_calculator::linear_cc<addresser_type_uv>(pos0, pos1, t, vec4(x, y, 0, 0),
				int4(static_cast<int>(surf.get_width()), static_cast<int>(surf.get_height()), 0, 0));

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
				point<addresser::wrap, addresser::wrap>::op,
				point<addresser::wrap, addresser::mirror>::op,
				point<addresser::wrap, addresser::clamp>::op,
				point<addresser::wrap, addresser::border>::op
			},
			{
				point<addresser::mirror, addresser::wrap>::op,
				point<addresser::mirror, addresser::mirror>::op,
				point<addresser::mirror, addresser::clamp>::op,
				point<addresser::mirror, addresser::border>::op
			},
			{
				point<addresser::clamp, addresser::wrap>::op,
				point<addresser::clamp, addresser::mirror>::op,
				point<addresser::clamp, addresser::clamp>::op,
				point<addresser::clamp, addresser::border>::op
			},
			{
				point<addresser::border, addresser::wrap>::op,
				point<addresser::border, addresser::mirror>::op,
				point<addresser::border, addresser::clamp>::op,
				point<addresser::border, addresser::border>::op
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
#ifndef EFLIB_NO_SIMD
	static const union
	{
		int maski;
		float maskf;
	} MASK = { 0x7FFFFFFF };
	static const __m128 ABS_MASK = _mm_set1_ps(MASK.maskf);

	__m128 mattr = _mm_loadu_ps(&attribute.x);
	__m128 tmp = _mm_mul_ps(mattr, _mm_set1_ps(inv_w));
	__m128 mddx2 = _mm_sub_ps(_mm_mul_ps(_mm_add_ps(mattr, _mm_loadu_ps(&ddx.x)), _mm_set1_ps(inv_x_w)), tmp);
	__m128 mddy2 = _mm_sub_ps(_mm_mul_ps(_mm_add_ps(mattr, _mm_loadu_ps(&ddy.x)), _mm_set1_ps(inv_y_w)), tmp);
	mddx2 = _mm_and_ps(mddx2, ABS_MASK);
	mddy2 = _mm_and_ps(mddy2, ABS_MASK);
	tmp = _mm_max_ps(mddx2, mddy2);
	tmp = _mm_mul_ps(tmp, _mm_loadu_ps(&size.x));
	vec4 maxD;
	_mm_storeu_ps(&maxD.x, tmp);

	float rho, lambda;
	rho = max(max(maxD.x, maxD.y), maxD.z);
	if(rho == 0.0f) rho = 0.000001f;
	lambda = fast_log2(rho);
	return lambda + bias;
#else
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
#endif
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
