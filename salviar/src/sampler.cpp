#include <salviar/include/sampler.h>

#include <salviar/include/surface.h>
#include <salviar/include/texture.h>

#include <eflib/include/platform/intrin.h>

BEGIN_NS_SALVIAR();

using namespace eflib;
using namespace std;

#define LOD_QUALITY_HI   2
#define LOD_QUALITY_MID  1
#define LOD_QUALITY_LOW  0

#define LOD_QUALITY LOD_QUALITY_MID

namespace addresser
{
	struct wrap
	{
		static float do_coordf(float coord, int size)
		{
			return (coord - fast_floor(coord)) * size - 0.5f;
		}

		static int do_coordi_point_1d(int coord, int size)
		{
			return (size * 8192 + coord) % size;
		}

		static int4 do_coordi_point_2d(const vec4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_loadu_ps(&coord[0]);
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size[0]));

			mfcoord = _mm_sub_ps(mfcoord, _mm_cvtepi32_ps(_mm_cvttps_epi32(mfcoord)));
			__m128 mfsize = _mm_cvtepi32_ps(misize);
			mfcoord = _mm_mul_ps(mfsize, mfcoord);

			__m128 mfcoord_ipart = _mm_cvtepi32_ps(_mm_cvttps_epi32(mfcoord));
			__m128 mask = _mm_cmpgt_ps(mfcoord_ipart, mfcoord);		// if it increased (i.e. if it was negative...)
			mask = _mm_and_ps(mask, _mm_set1_ps(1.0f));				// ...without a conditional branch...
			mfcoord_ipart = _mm_sub_ps(mfcoord_ipart, mask);

			mfcoord = _mm_add_ps(mfcoord_ipart, _mm_mul_ps(mfsize, _mm_set1_ps(8192.0f)));
			__m128 mfdiv = _mm_cvtepi32_ps(_mm_cvttps_epi32(_mm_div_ps(mfcoord, mfsize)));
			__m128i tmp = _mm_cvttps_epi32(_mm_sub_ps(mfcoord, _mm_mul_ps(mfdiv, mfsize)));
			int4 ret;
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&ret[0]), tmp);
			return ret;
#else
			vec4 o_coord = (coord - vec4(fast_floor(coord[0]), fast_floor(coord[1]), fast_floor(coord[2]), fast_floor(coord[3])))
				* vec4(static_cast<float>(size[0]), static_cast<float>(size[1]), static_cast<float>(size[2]), static_cast<float>(size[3]));
			int4 coord_ipart = int4(fast_floori(o_coord[0]), fast_floori(o_coord[1]), 0, 0);

			return int4((size[0] * 8192 + coord_ipart[0]) % size[0],
				(size[1] * 8192 + coord_ipart[1]) % size[1],
				0, 0);
#endif
		}

		static void do_coordi_linear_2d(int4& low, int4& up, vec4& frac, const vec4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_loadu_ps(&coord[0]);
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size[0]));

			__m128 mfcoord0 = _mm_sub_ps(mfcoord, _mm_cvtepi32_ps(_mm_cvttps_epi32(mfcoord)));
			misize = _mm_unpacklo_epi64(misize, misize);
			__m128 mfsize = _mm_cvtepi32_ps(misize);
			mfcoord0 = _mm_mul_ps(mfsize, mfcoord0);
			const __m128 mhalf = _mm_set1_ps(0.5f);
			mfcoord0 = _mm_sub_ps(mfcoord0, mhalf);

			__m128 mfcoord_ipart = _mm_cvtepi32_ps(_mm_cvttps_epi32(mfcoord0));
			__m128 mask = _mm_cmpgt_ps(mfcoord_ipart, mfcoord0);	// if it increased (i.e. if it was negative...)
			mask = _mm_and_ps(mask, _mm_set1_ps(1.0f));				// ...without a conditional branch...
			mfcoord_ipart = _mm_sub_ps(mfcoord_ipart, mask);
			__m128 mffrac = _mm_sub_ps(mfcoord0, mfcoord_ipart);
			_mm_storeu_ps(&frac[0], mffrac);

			__m128 mfcoord01 = _mm_movelh_ps(mfcoord_ipart, mfcoord_ipart);
			mfcoord01 = _mm_add_ps(mfcoord01, _mm_set_ps(1.0f, 1.0f, 0.0f, 0.0f));
			mfcoord01 = _mm_add_ps(mfcoord01, _mm_mul_ps(mfsize, _mm_set1_ps(8192.0f)));
			__m128 mfdiv = _mm_cvtepi32_ps(_mm_cvttps_epi32(_mm_div_ps(mfcoord01, mfsize)));
			__m128i tmp = _mm_cvttps_epi32(_mm_sub_ps(mfcoord01, _mm_mul_ps(mfdiv, mfsize)));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&low[0]), tmp);
			tmp = _mm_unpackhi_epi64(tmp, tmp);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&up[0]), tmp);
#else
			vec4 o_coord = (coord - vec4(fast_floor(coord[0]), fast_floor(coord[1]), fast_floor(coord[2]), fast_floor(coord[3])))
				* vec4(static_cast<float>(size[0]), static_cast<float>(size[1]), static_cast<float>(size[2]), static_cast<float>(size[3])) - 0.5f;
			int4 coord_ipart = int4(fast_floori(o_coord[0]), fast_floori(o_coord[1]), 0, 0);
			frac = o_coord - vec4(static_cast<float>(coord_ipart[0]), static_cast<float>(coord_ipart[1]), 0, 0);

			low = int4((size[0] * 8192 + coord_ipart[0]) % size[0],
				(size[1] * 8192 + coord_ipart[1]) % size[1],
				0, 0);
			up = int4((size[0] * 8192 + coord_ipart[0] + 1) % size[0],
				(size[1] * 8192 + coord_ipart[1] + 1) % size[1],
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

		static int do_coordi_point_1d(int coord, int size)
		{
			return eflib::clamp(coord, 0, size - 1);
		}
		static int4 do_coordi_point_2d(const vec4& coord, const int4& size)
		{
			int selection_coord_x = fast_floori(coord[0]);
			int selection_coord_y = fast_floori(coord[1]);
			vec4 o_coord((selection_coord_x & 1
				? 1 + selection_coord_x - coord[0]
			: coord[0] - selection_coord_x) * size[0],
				(selection_coord_y & 1
				? 1 + selection_coord_y - coord[1]
			: coord[1] - selection_coord_y) * size[1],
				0, 0);

#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_loadu_ps(&o_coord[0]);
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size));

			__m128 mfcoord_ipart = _mm_cvtepi32_ps(_mm_cvttps_epi32(mfcoord));
			__m128 mask = _mm_cmpgt_ps(mfcoord_ipart, mfcoord);		// if it increased (i.e. if it was negative...)
			mask = _mm_and_ps(mask, _mm_set1_ps(1.0f));				// ...without a conditional branch...
			mfcoord_ipart = _mm_sub_ps(mfcoord_ipart, mask);

			__m128 mfsize = _mm_cvtepi32_ps(misize);
			mfsize = _mm_sub_ps(mfsize, _mm_set1_ps(1.0f));
			__m128i tmp = _mm_cvttps_epi32(_mm_min_ps(_mm_max_ps(mfcoord_ipart, _mm_setzero_ps()), mfsize));
			int4 ret;
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&ret[0]), tmp);
			return ret;
#else
			int4 coord_ipart = int4(fast_floori(o_coord[0]), fast_floori(o_coord[1]), 0, 0);

			return int4(eflib::clamp(coord_ipart[0], 0, size[0] - 1),
				eflib::clamp(coord_ipart[1], 0, size[1] - 1),
				0, 0);
#endif
		}

		static void do_coordi_linear_2d(int4& low, int4& up, vec4& frac, const vec4& coord, const int4& size)
		{
			int selection_coord_x = fast_floori(coord[0]);
			int selection_coord_y = fast_floori(coord[1]);
			vec4 o_coord((selection_coord_x & 1
				? 1 + selection_coord_x - coord[0]
			: coord[0] - selection_coord_x) * size[0] - 0.5f,
				(selection_coord_y & 1
				? 1 + selection_coord_y - coord[1]
			: coord[1] - selection_coord_y) * size[1] - 0.5f,
				0, 0);

#ifndef EFLIB_NO_SIMD
			__m128 mfcoord0 = _mm_loadu_ps(&o_coord[0]);
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size));

			__m128 mfcoord_ipart = _mm_cvtepi32_ps(_mm_cvttps_epi32(mfcoord0));
			__m128 mask = _mm_cmpgt_ps(mfcoord_ipart, mfcoord0);	// if it increased (i.e. if it was negative...)
			mask = _mm_and_ps(mask, _mm_set1_ps(1.0f));				// ...without a conditional branch...
			mfcoord_ipart = _mm_sub_ps(mfcoord_ipart, mask);
			__m128 mffrac = _mm_sub_ps(mfcoord0, mfcoord_ipart);
			_mm_storeu_ps(&frac[0], mffrac);

			__m128 mfcoord01 = _mm_movelh_ps(mfcoord_ipart, mfcoord_ipart);
			mfcoord01 = _mm_add_ps(mfcoord01, _mm_set_ps(1.0f, 1.0f, 0.0f, 0.0f));
			__m128 mfsize = _mm_cvtepi32_ps(misize);
			mfsize = _mm_movelh_ps(mfsize, mfsize);
			mfsize = _mm_sub_ps(mfsize, _mm_set1_ps(1.0f));
			__m128i tmp = _mm_cvttps_epi32(_mm_min_ps(_mm_max_ps(mfcoord01, _mm_setzero_ps()), mfsize));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&low[0]), tmp);
			tmp = _mm_unpackhi_epi64(tmp, tmp);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&up[0]), tmp);
#else
			int4 coord_ipart = int4(fast_floori(o_coord[0]), fast_floori(o_coord[1]), 0, 0);
			frac = o_coord - vec4(static_cast<float>(coord_ipart[0]), static_cast<float>(coord_ipart[1]), 0, 0);

			low = int4(eflib::clamp(coord_ipart[0], 0, size[0] - 1),
				eflib::clamp(coord_ipart[1], 0, size[1] - 1),
				0, 0);
			up = int4(eflib::clamp(coord_ipart[0] + 1, 0, size[0] - 1),
				eflib::clamp(coord_ipart[1] + 1, 0, size[1] - 1),
				0, 0);
#endif
		}
	};

	struct clamp
	{
		static float do_coordf(float coord, int size)
		{
			return eflib::clamp(coord * size, 0.5f, size - 0.5f) - 0.5f;
		}

		static int do_coordi_point_1d(int coord, int size)
		{
			return eflib::clamp(coord, 0, size - 1);
		}
		static int4 do_coordi_point_2d(const vec4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_loadu_ps(&coord[0]);
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size[0]));

			misize = _mm_unpacklo_epi64(misize, misize);
			__m128 mfsize = _mm_cvtepi32_ps(misize);
			const __m128 mhalf = _mm_set1_ps(0.5f);
			__m128 mfcoord0 = _mm_mul_ps(mfcoord, mfsize);
			mfcoord0 = _mm_max_ps(mfcoord0, mhalf);
			mfcoord0 = _mm_min_ps(mfcoord0, _mm_sub_ps(mfsize, mhalf));

			__m128 mfcoord0_ipart = _mm_cvtepi32_ps(_mm_cvttps_epi32(mfcoord0));
			__m128 mask = _mm_cmpgt_ps(mfcoord0_ipart, mfcoord0);		// if it increased (i.e. if it was negative...)
			mask = _mm_and_ps(mask, _mm_set1_ps(1.0f));				// ...without a conditional branch...
			mfcoord0_ipart = _mm_sub_ps(mfcoord0_ipart, mask);

			mfsize = _mm_sub_ps(mfsize, _mm_set1_ps(1));
			__m128i tmp = _mm_cvttps_epi32(_mm_min_ps(_mm_max_ps(mfcoord0_ipart, _mm_setzero_ps()), mfsize));
			int4 ret;
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&ret[0]), tmp);
			return ret;
#else
			vec4 o_coord(eflib::clamp(coord[0] * size[0], 0.5f, size[0] - 0.5f),
				eflib::clamp(coord[1] * size[1], 0.5f, size[1] - 0.5f),
				0, 0);
			int4 coord_ipart = int4(fast_floori(o_coord[0]), fast_floori(o_coord[1]), 0, 0);

			return int4(eflib::clamp(coord_ipart[0], 0, size[0] - 1),
				eflib::clamp(coord_ipart[1], 0, size[1] - 1),
				0, 0);
#endif
		}

		static void do_coordi_linear_2d(int4& low, int4& up, vec4& frac, const vec4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_loadu_ps(&coord[0]);
			__m128i misize = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&size[0]));

			misize = _mm_unpacklo_epi64(misize, misize);
			__m128 mfsize = _mm_cvtepi32_ps(misize);
			const __m128 mhalf = _mm_set1_ps(0.5f);
			__m128 mfcoord0 = _mm_mul_ps(mfcoord, mfsize);
			mfcoord0 = _mm_max_ps(mfcoord0, mhalf);
			mfcoord0 = _mm_min_ps(mfcoord0, _mm_sub_ps(mfsize, mhalf));
			mfcoord0 = _mm_sub_ps(mfcoord0, mhalf);

			__m128 mfcoord_ipart = _mm_cvtepi32_ps(_mm_cvttps_epi32(mfcoord0));
			__m128 mask = _mm_cmpgt_ps(mfcoord_ipart, mfcoord0);	// if it increased (i.e. if it was negative...)
			mask = _mm_and_ps(mask, _mm_set1_ps(1.0f));				// ...without a conditional branch...
			mfcoord_ipart = _mm_sub_ps(mfcoord_ipart, mask);
			__m128 mffrac = _mm_sub_ps(mfcoord0, mfcoord_ipart);
			_mm_storeu_ps(&frac[0], mffrac);

			__m128 mfcoord01 = _mm_movelh_ps(mfcoord_ipart, mfcoord_ipart);
			mfcoord01 = _mm_add_ps(mfcoord01, _mm_set_ps(1.0f, 1.0f, 0.0f, 0.0f));
			mfsize = _mm_sub_ps(mfsize, _mm_set1_ps(1));
			__m128i tmp = _mm_cvttps_epi32(_mm_min_ps(_mm_max_ps(mfcoord01, _mm_setzero_ps()), mfsize));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&low[0]), tmp);
			tmp = _mm_unpackhi_epi64(tmp, tmp);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&up[0]), tmp);
#else
			vec4 o_coord(eflib::clamp(coord[0] * size[0], 0.5f, size[0] - 0.5f) - 0.5f,
				eflib::clamp(coord[1] * size[1], 0.5f, size[1] - 0.5f) - 0.5f,
				0, 0);
			int4 coord_ipart = int4(fast_floori(o_coord[0]), fast_floori(o_coord[1]), 0, 0);
			frac = o_coord - vec4(static_cast<float>(coord_ipart[0]), static_cast<float>(coord_ipart[1]), 0, 0);

			low = int4(eflib::clamp(coord_ipart[0], 0, size[0] - 1),
				eflib::clamp(coord_ipart[1], 0, size[1] - 1),
				0, 0);
			up = int4(eflib::clamp(coord_ipart[0] + 1, 0, size[0] - 1),
				eflib::clamp(coord_ipart[1] + 1, 0, size[1] - 1),
				0, 0);
#endif
		}
	};

	struct border
	{
		static float do_coordf(float coord, int size)
		{
			return eflib::clamp(coord * size, -0.5f, size + 0.5f) - 0.5f;
		}

		static int do_coordi_point_1d(int coord, int size)
		{
			return coord >= size ? -1 : coord;
		}
		static int4 do_coordi_point_2d(const vec4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_loadu_ps(&coord[0]);
			__m128 mfsize = _mm_cvtepi32_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&size[0])));

			const __m128 mneghalf = _mm_set1_ps(-0.5f);
			__m128 tmp = _mm_mul_ps(mfcoord, mfsize);
			tmp = _mm_max_ps(tmp, mneghalf);
			mfcoord = _mm_min_ps(tmp, _mm_sub_ps(mfsize, mneghalf));

			__m128 mfcoord_ipart = _mm_cvtepi32_ps(_mm_cvttps_epi32(mfcoord));
			__m128 mask = _mm_cmpgt_ps(mfcoord_ipart, mfcoord);		// if it increased (i.e. if it was negative...)
			mask = _mm_and_ps(mask, _mm_set1_ps(1.0f));				// ...without a conditional branch...
			mfcoord_ipart = _mm_sub_ps(mfcoord_ipart, mask);
			int4 coord_ipart;
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&coord_ipart[0]), _mm_cvttps_epi32(mfcoord_ipart));
#else
			vec4 o_coord(eflib::clamp(coord[0] * size[0], -0.5f, size[0] + 0.5f),
				eflib::clamp(coord[1] * size[1], -0.5f, size[1] + 0.5f),
				0, 0);

			int4 coord_ipart = int4(fast_floori(o_coord[0]), fast_floori(o_coord[1]), 0, 0);
#endif

			return int4(coord_ipart[0] >= size[0] ? -1 : coord_ipart[0],
				coord_ipart[1] >= size[1] ? -1 : coord_ipart[1],
				0, 0);
		}

		static void do_coordi_linear_2d(int4& low, int4& up, vec4& frac, const vec4& coord, const int4& size)
		{
#ifndef EFLIB_NO_SIMD
			__m128 mfcoord = _mm_loadu_ps(&coord[0]);
			__m128 mfsize = _mm_cvtepi32_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&size[0])));

			const __m128 mneghalf = _mm_set1_ps(-0.5f);
			__m128 tmp = _mm_mul_ps(mfcoord, mfsize);
			tmp = _mm_max_ps(tmp, mneghalf);
			tmp = _mm_min_ps(tmp, _mm_sub_ps(mfsize, mneghalf));
			__m128 mfcoord0 = _mm_add_ps(tmp, mneghalf);

			__m128 mfcoord_ipart = _mm_cvtepi32_ps(_mm_cvttps_epi32(mfcoord0));
			__m128 mask = _mm_cmpgt_ps(mfcoord_ipart, mfcoord0);	// if it increased (i.e. if it was negative...)
			mask = _mm_and_ps(mask, _mm_set1_ps(1.0f));				// ...without a conditional branch...
			mfcoord_ipart = _mm_sub_ps(mfcoord_ipart, mask);
			__m128 mffrac = _mm_sub_ps(mfcoord0, mfcoord_ipart);
			_mm_storeu_ps(&frac[0], mffrac);
			int4 coord_ipart;
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&coord_ipart[0]), _mm_cvttps_epi32(mfcoord_ipart));
#else
			vec4 o_coord(eflib::clamp(coord[0] * size[0], -0.5f, size[0] + 0.5f) - 0.5f,
				eflib::clamp(coord[1] * size[1], -0.5f, size[1] + 0.5f) - 0.5f,
				0, 0);
			int4 coord_ipart = int4(fast_floori(o_coord[0]), fast_floori(o_coord[1]), 0, 0);
			frac = o_coord - vec4(static_cast<float>(coord_ipart[0]), static_cast<float>(coord_ipart[1]), 0, 0);
#endif

			low = int4(coord_ipart[0] >= size[0] ? -1 : coord_ipart[0],
				coord_ipart[1] >= size[1] ? -1 : coord_ipart[1],
				0, 0);
			up = int4(coord_ipart[0] + 1 >= size[0] ? -1 : coord_ipart[0] + 1,
				coord_ipart[1] + 1 >= size[1] ? -1 : coord_ipart[1] + 1,
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
		return addresser_type::do_coordi_point_2d(coord, size);
	}

	template <typename addresser_type>
	void linear_cc(int4& low, int4& up, vec4& frac, const vec4& coord, const int4& size)
	{
		addresser_type::do_coordi_linear_2d(low, up, frac, coord, size);
	}
};

namespace surface_sampler
{
	template <typename addresser_type_u, typename addresser_type_v>
	struct point
	{
		static color_rgba32f op(const surface& surf, float x, float y, size_t sample, const color_rgba32f& border_color)
		{
			int ix = coord_calculator::point_cc<addresser_type_u>(x, int(surf.width()));
			int iy = coord_calculator::point_cc<addresser_type_v>(y, int(surf.height()));

			if(ix < 0 || iy < 0) return border_color;
			return surf.get_texel(ix, iy, sample);
		}
	};

	template <typename addresser_type_u, typename addresser_type_v>
	struct linear
	{
		static color_rgba32f op(const surface& surf, float x, float y, size_t sample, const color_rgba32f& /*border_color*/)
		{
			int xpos0, ypos0, xpos1, ypos1;
			float tx, ty;
			coord_calculator::linear_cc<addresser_type_u>(xpos0, xpos1, tx, x, int(surf.width()));
			coord_calculator::linear_cc<addresser_type_v>(ypos0, ypos1, ty, y, int(surf.height()));

			return surf.get_texel(xpos0, ypos0, xpos1, ypos1, tx, ty, sample);
		}
	};

	template <typename addresser_type_uv>
	struct point<addresser_type_uv, addresser_type_uv>
	{
		static color_rgba32f op(const surface& surf, float x, float y, size_t sample, const color_rgba32f& border_color)
		{
			int4 region_size(static_cast<int>(surf.width()), static_cast<int>(surf.height()), 0, 0);
			int4 ixy = coord_calculator::point_cc<addresser_type_uv>(vec4(x, y, 0, 0), region_size);

			if( 0 <= ixy[0] && ixy[0] < region_size[0] && 0 <= ixy[1] && ixy[1] < region_size[1] )
			{
				return surf.get_texel(ixy[0], ixy[1], sample);
			}

			return border_color;
		}
	};

	template <typename addresser_type_uv>
	struct linear<addresser_type_uv, addresser_type_uv>
	{
		static color_rgba32f op(const surface& surf, float x, float y, size_t sample, const color_rgba32f& /*border_color*/)
		{
			int4 pos0, pos1;
			vec4 t;
			coord_calculator::linear_cc<addresser_type_uv>(pos0, pos1, t, vec4(x, y, 0, 0),
				int4(static_cast<int>(surf.width()), static_cast<int>(surf.height()), 0, 0));

			// printf("(%d, %d) - (%d, %d): (%0.3f, %0.3f)\n", pos0[0], pos0[1], pos1[0], pos1[1], t[0], t[1]);

			return surf.get_texel(pos0[0], pos0[1], pos1[0], pos1[1], t[0], t[1], sample);
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

float sampler::calc_lod( eflib::uint4 const& size, eflib::vec4 const& ddx, eflib::vec4 const& ddy, float bias ) const
{
#if LOD_QUALITY == LOD_QUALITY_LOW
#	if 0 && !defined(EFLIB_NO_SIMD)
	static const union
	{
		int maski;
		float maskf;
	} MASK = { 0x7FFFFFFF };
	static const __m128 ABS_MASK = _mm_set1_ps(MASK.maskf);

	__m128 mddx = _mm_loadu_ps(&ddx[0]);
	__m128 mddy = _mm_loadu_ps(&ddy[0]);
	mddx = _mm_and_ps(mddx, ABS_MASK);
	mddy = _mm_and_ps(mddy, ABS_MASK);
	__m128 mmax = _mm_max_ps(mddx, mddy);
	mmax = _mm_mul_ps(mmax, _mm_cvtepi32_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&size[0]))));
	__m128 mmax2 = _mm_shuffle_ps(mmax, mmax, _MM_SHUFFLE(2, 2, 2, 2));
	mmax = _mm_max_ps(mmax, mmax2);
	mmax2 = _mm_shuffle_ps(mmax, mmax, _MM_SHUFFLE(1, 1, 1, 1));
	mmax = _mm_max_ss(mmax, mmax2);
	__m128 mrho = _mm_max_ss(mmax, _mm_set_ss(0.000001f));

	// log 2
	__m128i mx = _mm_castps_si128(mrho);
	__m128 mlog2 = _mm_cvtepi32_ps(_mm_sub_epi32(_mm_and_si128(_mm_srli_epi32(mx, 23), _mm_set1_epi32(255)), _mm_set1_epi32(128)));
	mx = _mm_and_si128(mx, _mm_set1_epi32(0x007FFFFF));
	mx = _mm_or_si128(mx, _mm_set1_epi32(0x3F800000));
	__m128 tmp = _mm_castsi128_ps(mx);
	tmp = _mm_sub_ss(_mm_mul_ss(_mm_add_ss(_mm_mul_ss(tmp, _mm_set_ss(-1.0f / 3)), _mm_set_ss(2)), tmp), _mm_set_ss(2.0f / 3));
	__m128 mlambda = _mm_add_ss(tmp, mlog2);

	__m128 mlod = _mm_add_ss(mlambda, _mm_set_ss(bias));

	float lod;
	_mm_store_ss(&lod, mlod);
	return lod;
#	else
	vec4 size_vec4( static_cast<float>(size[0]), static_cast<float>(size[1]), static_cast<float>(size[2]), 0 );
	
	vec4 maxD(
		max(abs(ddx[0]), abs(ddy[0])),
		max(abs(ddx[1]), abs(ddy[1])),
		max(abs(ddx[2]), abs(ddy[2])),
		0.0f);
	maxD *= vec4(static_cast<float>(size[0]), static_cast<float>(size[1]), static_cast<float>(size[2]), 0);
	float rho = max(max(maxD[0], maxD[1]), maxD[2]);
	float lambda = fast_log2(rho);
	return lambda + bias;
#	endif
#else
	vec4 size_vec4( static_cast<float>(size[0]), static_cast<float>(size[1]), static_cast<float>(size[2]), 0 );
	vec4 ddx_ts = ddx * size_vec4;										// (1, 0)
	vec4 ddy_ts = ddy * size_vec4;										// (0, 1)
	float rho, lambda;

#	if LOD_QUALITY == LOD_QUALITY_HI
	auto A = ddx_ts[0] * ddx_ts[0] + ddy_ts[0] * ddy_ts[0];				// 1
	auto B = -2.0f * (ddx_ts[0] * ddx_ts[1] + ddy_ts[0] * ddy_ts[1]);	// 0
	auto C = ddx_ts[1] * ddx_ts[1] + ddy_ts[1] * ddy_ts[1];				// 1
	auto F = A * C - B * B * 0.25f;										// 1
	auto invF = 1.0f / F;
	A *= invF; B *= invF; C *= invF;
	auto AsubC = A - C;													// 0
	auto R = sqrt(AsubC * AsubC + B * B);								// 0
	rho = sqrt( 2.0f / (A + C - R) );									// 1
#	else
	static_assert(LOD_QUALITY == LOD_QUALITY_MID, "");
	float ddx_rho = ddx_ts.xyz().length();
	float ddy_rho = ddy_ts.xyz().length();
	rho = max(ddx_rho, ddy_rho);
#	endif

	if(rho == 0.0f) rho = 0.000001f;
	lambda = fast_log2(rho);
	return lambda + bias;
#endif
}

color_rgba32f sampler::sample_surface(
	const surface& surf,
	float x, float y, size_t sample,
	sampler_state ss) const
{
	auto ret = filters_[ss](
		surf,
		x, y, sample,
		desc_.border_color
		);
	return ret;
}

sampler::sampler(sampler_desc const& desc, texture_ptr const& tex)
	: desc_(desc)
	, tex_(tex)
{
	filters_[sampler_state_min] = surface_sampler::filter_table[desc_.min_filter][desc_.addr_mode_u][desc.addr_mode_v];
	filters_[sampler_state_mag] = surface_sampler::filter_table[desc_.mag_filter][desc_.addr_mode_u][desc.addr_mode_v];
	filters_[sampler_state_mip] = surface_sampler::filter_table[desc_.mip_filter][desc_.addr_mode_u][desc.addr_mode_v];
}

inline size_t compute_cube_subresource(std::true_type, size_t face, size_t lod_level)
{
	return lod_level * 6 + face;
}

inline size_t compute_cube_subresource(std::false_type, size_t /*face*/, size_t lod_level)
{
	return lod_level;
}

template <bool IsCubeTexture>
color_rgba32f sampler::sample_impl(int face, float coordx, float coordy, size_t sample, float miplevel, float ratio, vec4 const& long_axis) const
{
	std::integral_constant<bool, IsCubeTexture> dummy;
	size_t face_sz = static_cast<size_t>(face);

	bool is_mag
		= (desc_.mip_filter == filter_point)
		? (miplevel < 0.5f)
		: (miplevel < 0.0f)
		;

	if(is_mag)
	{
		auto subres_index = compute_cube_subresource(dummy, face_sz, tex_->max_lod() );
		return sample_surface(*tex_->subresource(subres_index), coordx, coordy, sample, sampler_state_mag);
	}

	if(desc_.mip_filter == filter_point)
	{
		int ml = fast_floori(miplevel + 0.5f);

		ml = clamp(ml, static_cast<int>(tex_->max_lod()), static_cast<int>(tex_->min_lod()));

		auto subres_index = compute_cube_subresource(dummy, face_sz, ml);
		return sample_surface(*tex_->subresource(subres_index), coordx, coordy, sample, sampler_state_min);
	}

	if(desc_.mip_filter == filter_linear)
	{
		int lo = fast_floori(miplevel);
		int hi = lo + 1;

		float frac = miplevel - lo;

		auto lo_sz = static_cast<size_t>(clamp(lo, static_cast<int>(tex_->max_lod()), static_cast<int>(tex_->min_lod())));
		auto hi_sz = static_cast<size_t>(clamp(hi, static_cast<int>(tex_->max_lod()), static_cast<int>(tex_->min_lod())));

		auto subres_index_lo = compute_cube_subresource(dummy, face_sz, lo_sz);
		auto subres_index_hi = compute_cube_subresource(dummy, face_sz, hi_sz);

		color_rgba32f c0 = sample_surface(*tex_->subresource(subres_index_lo), coordx, coordy, sample, sampler_state_min);
		color_rgba32f c1 = sample_surface(*tex_->subresource(subres_index_hi), coordx, coordy, sample, sampler_state_min);

		return lerp(c0, c1, frac);
	}

	if(desc_.mip_filter == filter_anisotropic)
	{
		float start_relative_distance = - 0.5f * (ratio - 1.0f);

		float sample_coord_x = coordx + long_axis.x() * start_relative_distance;
		float sample_coord_y = coordy + long_axis.y() * start_relative_distance;

		int lo = fast_floori(miplevel);
		int hi  = lo + 1;

		float frac = miplevel - lo;
		lo = max(lo, 0);
		hi = max(hi, 0);

		auto lo_sz = clamp(static_cast<size_t>(lo), tex_->max_lod(), tex_->min_lod());
		auto hi_sz = clamp(static_cast<size_t>(hi), tex_->max_lod(), tex_->min_lod());
		EFLIB_UNREF_DECLARATOR(hi_sz);

		vec4 color(0.0f, 0.0f, 0.0f, 0.0f);
		for(int i_sample = 0; i_sample < static_cast<int>(ratio); ++i_sample)
		{
			auto subres_index_lo = compute_cube_subresource(dummy, face_sz, lo_sz);
			auto subres_index_hi = compute_cube_subresource(dummy, face_sz, hi_sz);
			color_rgba32f c0 = sample_surface(
				*tex_->subresource(subres_index_lo), sample_coord_x, sample_coord_y, sample, sampler_state_min
				);
			color_rgba32f c1 = sample_surface(
				*tex_->subresource(subres_index_hi), sample_coord_x, sample_coord_y, sample, sampler_state_min
				);

			color += lerp(c0, c1, frac).get_vec4();

			sample_coord_x += long_axis.x();
			sample_coord_y += long_axis.y();
		}

		color /= ratio;

		return color_rgba32f(color);
	}

	EFLIB_ASSERT(false, "Mip filters is error.");
	return desc_.border_color;
}

color_rgba32f sampler::sample(float coordx, float coordy, float miplevel) const
{
	return sample_impl<false>(0, coordx, coordy, 0, miplevel, 1.0f, vec4(0.0f, 0.0f, 0.0f, 0.0f));
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

	EFLIB_ASSERT(tex_->get_texture_type() != texture_type_cube , "texture is not a cube texture.");

	return sample_impl<true>(major_dir, s, t, 0, miplevel, 1.0f, vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

float sampler::calc_lod_2d(eflib::vec2 const& ddx, eflib::vec2 const& ddy) const
{
	uint4 size = tex_->size();

	vec4 ddx_vec4(ddx[0], ddx[1], 0.0f, 0.0f);
	vec4 ddy_vec4(ddy[0], ddy[1], 0.0f, 0.0f);

	float lod, ratio;
	vec4  long_axis;
	if( desc_.mip_filter == filter_anisotropic && desc_.max_anisotropy > 1 )
	{
		calc_anisotropic_lod(size, ddx_vec4, ddy_vec4, 0.0f, lod, ratio, long_axis);
	}
	else
	{
		lod = calc_lod(size, ddx_vec4, ddy_vec4, 0.0f);
		ratio = 1.0f;
	}

	return lod;
}

color_rgba32f sampler::sample_2d_lod( eflib::vec2 const& proj_coord, float lod ) const
{
	return sample( proj_coord[0], proj_coord[1], lod );
}

color_rgba32f sampler::sample_2d_grad( eflib::vec2 const& proj_coord, eflib::vec2 const& ddx, eflib::vec2 const& ddy, float lod_bias ) const
{
	uint4 size = tex_->size();

	vec4 ddx_vec4(ddx[0], ddx[1], 0.0f, 0.0f);
	vec4 ddy_vec4(ddy[0], ddy[1], 0.0f, 0.0f);

	float lod, ratio;
	vec4  long_axis;
	if( desc_.mip_filter == filter_anisotropic && desc_.max_anisotropy > 1 )
	{
		calc_anisotropic_lod(size, ddx_vec4, ddy_vec4, lod_bias, lod, ratio, long_axis);
	}
	else
	{
		lod = calc_lod(size, ddx_vec4, ddy_vec4, lod_bias);
		ratio = 1.0f;
	}

	return sample_impl<false>(0, proj_coord[0], proj_coord[1], 0, lod, ratio, long_axis);
}

void sampler::calc_anisotropic_lod(
	eflib::uint4 const& size,
	eflib::vec4 const& ddx, eflib::vec4 const& ddy, float bias,
	float& out_lod, float& out_ratio, vec4& out_long_axis ) const
{
	vec4 size_vec4( static_cast<float>(size[0]), static_cast<float>(size[1]), static_cast<float>(size[2]), 0 );

	vec4 ddx_ts = ddx * size_vec4;
	vec4 ddy_ts = ddy * size_vec4;

	float ddx_len = ddx_ts.xyz().length();
	float ddy_len = ddy_ts.xyz().length();

	float diag0_len = (ddx_ts - ddy_ts).xyz().length();
	float diag1_len = (ddx_ts + ddy_ts).xyz().length();

	auto minor_axis_len = min( min(diag0_len, diag1_len), min(ddx_len, ddy_len) );
	
	if(minor_axis_len == 0.0f) minor_axis_len = 0.000001f;

	vec4 const* long_axis;
	float long_axis_len;
	if(ddx_len > ddy_len)
	{
		long_axis_len = ddx_len;
		long_axis = &ddx_ts;
	}
	else
	{
		long_axis_len = ddy_len;
		long_axis = &ddy_ts;
	}

	float probe_count = long_axis_len / minor_axis_len;

	if ( probe_count > static_cast<float>(desc_.max_anisotropy) )
	{
		probe_count = static_cast<float>(desc_.max_anisotropy);	
	}
	else
	{
		probe_count = fast_round(probe_count);
	}

	out_ratio = probe_count;
	if (probe_count == 1.0f)
	{
		out_long_axis.xyzw(0.0f, 0.0f, 0.0f, 0.0f);
		out_lod = fast_log2(long_axis_len) + bias;
		return;
	}
	else
	{
		float step = long_axis_len / probe_count;
		minor_axis_len = 2 * long_axis_len / (probe_count + 1);
		out_lod = fast_log2(minor_axis_len) + bias;
		out_long_axis = (*long_axis) * step / long_axis_len;
		out_long_axis *= (1.0f / size_vec4);
	}
}

END_NS_SALVIAR();
