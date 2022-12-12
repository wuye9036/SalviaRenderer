#include <salviar/include/sampler.h>

#include <salviar/include/surface.h>
#include <salviar/include/texture.h>

#include <eflib/platform/intrin.h>

namespace salviar{


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
	vec4 size_vec4(static_cast<float>(size[0]), static_cast<float>(size[1]), static_cast<float>(size[2]), 0);

	if (desc_.mip_qual == mip_lo_quality)
	{
#if 0 && !defined(EFLIB_NO_SIMD)
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
#else
		vec4 maxD(
			max(abs(ddx[0]), abs(ddy[0])),
			max(abs(ddx[1]), abs(ddy[1])),
			max(abs(ddx[2]), abs(ddy[2])),
			0.0f);
		maxD *= size_vec4;
		float rho = max(max(maxD[0], maxD[1]), maxD[2]);
		float lambda = fast_log2(rho);
		return lambda + bias;
#endif
	}
	else
	{
		vec4 ddx_ts = ddx * size_vec4;										// (1, 0)
		vec4 ddy_ts = ddy * size_vec4;										// (0, 1)
		float rho, lambda;

		if (desc_.mip_qual == mip_hi_quality)
		{
			auto A = ddx_ts[0] * ddx_ts[0] + ddy_ts[0] * ddy_ts[0];				// 1
			auto B = -2.0f * (ddx_ts[0] * ddx_ts[1] + ddy_ts[0] * ddy_ts[1]);	// 0
			auto C = ddx_ts[1] * ddx_ts[1] + ddy_ts[1] * ddy_ts[1];				// 1
			auto F = A * C - B * B * 0.25f;										// 1
			auto invF = 1.0f / F;
			A *= invF; B *= invF; C *= invF;
			auto AsubC = A - C;													// 0
			auto R = sqrt(AsubC * AsubC + B * B);								// 0

			// Use major radius of EWA kernel shape as mip radius
			rho = sqrt(2.0f / (A + C - R));										// 1
		}
		else
		{
			float ddx_rho = ddx_ts.xyz().length();
			float ddy_rho = ddy_ts.xyz().length();
			rho = max(ddx_rho, ddy_rho);
		}

		if (rho == 0.0f) rho = 0.000001f;
		lambda = fast_log2(rho);
		return lambda + bias;
	}
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

// The table EWA weights is from blender
/**************************************************************************
 * Filtering method based on
 * "Creating raster omnimax images from multiple perspective views
 * using the elliptical weighted average filter"
 * by Ned Greene and Paul S. Heckbert (1986)
 ***************************************************************************/

 /* Table of (exp(ar) - exp(a)) / (1 - exp(a)) for r in range [0, 1] and a = -2
  * used instead of actual gaussian,
  * otherwise at high texture magnifications circular artifacts are visible. */
#define EWA_MAXIDX 255
const float EWA_WTS[EWA_MAXIDX + 1] = {
	1.f,         0.990965f,   0.982f,      0.973105f,   0.96428f,    0.955524f,   0.946836f,
	0.938216f,   0.929664f,   0.921178f,   0.912759f,   0.904405f,   0.896117f,   0.887893f,
	0.879734f,   0.871638f,   0.863605f,   0.855636f,   0.847728f,   0.839883f,   0.832098f,
	0.824375f,   0.816712f,   0.809108f,   0.801564f,   0.794079f,   0.786653f,   0.779284f,
	0.771974f,   0.76472f,    0.757523f,   0.750382f,   0.743297f,   0.736267f,   0.729292f,
	0.722372f,   0.715505f,   0.708693f,   0.701933f,   0.695227f,   0.688572f,   0.68197f,
	0.67542f,    0.66892f,    0.662471f,   0.656073f,   0.649725f,   0.643426f,   0.637176f,
	0.630976f,   0.624824f,   0.618719f,   0.612663f,   0.606654f,   0.600691f,   0.594776f,
	0.588906f,   0.583083f,   0.577305f,   0.571572f,   0.565883f,   0.56024f,    0.55464f,
	0.549084f,   0.543572f,   0.538102f,   0.532676f,   0.527291f,   0.521949f,   0.516649f,
	0.511389f,   0.506171f,   0.500994f,   0.495857f,   0.490761f,   0.485704f,   0.480687f,
	0.475709f,   0.470769f,   0.465869f,   0.461006f,   0.456182f,   0.451395f,   0.446646f,
	0.441934f,   0.437258f,   0.432619f,   0.428017f,   0.42345f,    0.418919f,   0.414424f,
	0.409963f,   0.405538f,   0.401147f,   0.39679f,    0.392467f,   0.388178f,   0.383923f,
	0.379701f,   0.375511f,   0.371355f,   0.367231f,   0.363139f,   0.359079f,   0.355051f,
	0.351055f,   0.347089f,   0.343155f,   0.339251f,   0.335378f,   0.331535f,   0.327722f,
	0.323939f,   0.320186f,   0.316461f,   0.312766f,   0.3091f,     0.305462f,   0.301853f,
	0.298272f,   0.294719f,   0.291194f,   0.287696f,   0.284226f,   0.280782f,   0.277366f,
	0.273976f,   0.270613f,   0.267276f,   0.263965f,   0.26068f,    0.257421f,   0.254187f,
	0.250979f,   0.247795f,   0.244636f,   0.241502f,   0.238393f,   0.235308f,   0.232246f,
	0.229209f,   0.226196f,   0.223206f,   0.220239f,   0.217296f,   0.214375f,   0.211478f,
	0.208603f,   0.20575f,    0.20292f,    0.200112f,   0.197326f,   0.194562f,   0.191819f,
	0.189097f,   0.186397f,   0.183718f,   0.18106f,    0.178423f,   0.175806f,   0.17321f,
	0.170634f,   0.168078f,   0.165542f,   0.163026f,   0.16053f,    0.158053f,   0.155595f,
	0.153157f,   0.150738f,   0.148337f,   0.145955f,   0.143592f,   0.141248f,   0.138921f,
	0.136613f,   0.134323f,   0.132051f,   0.129797f,   0.12756f,    0.125341f,   0.123139f,
	0.120954f,   0.118786f,   0.116635f,   0.114501f,   0.112384f,   0.110283f,   0.108199f,
	0.106131f,   0.104079f,   0.102043f,   0.100023f,   0.0980186f,  0.09603f,    0.094057f,
	0.0920994f,  0.0901571f,  0.08823f,    0.0863179f,  0.0844208f,  0.0825384f,  0.0806708f,
	0.0788178f,  0.0769792f,  0.0751551f,  0.0733451f,  0.0715493f,  0.0697676f,  0.0679997f,
	0.0662457f,  0.0645054f,  0.0627786f,  0.0610654f,  0.0593655f,  0.0576789f,  0.0560055f,
	0.0543452f,  0.0526979f,  0.0510634f,  0.0494416f,  0.0478326f,  0.0462361f,  0.0446521f,
	0.0430805f,  0.0415211f,  0.039974f,   0.0384389f,  0.0369158f,  0.0354046f,  0.0339052f,
	0.0324175f,  0.0309415f,  0.029477f,   0.0280239f,  0.0265822f,  0.0251517f,  0.0237324f,
	0.0223242f,  0.020927f,   0.0195408f,  0.0181653f,  0.0168006f,  0.0154466f,  0.0141031f,
	0.0127701f,  0.0114476f,  0.0101354f,  0.00883339f, 0.00754159f, 0.00625989f, 0.00498819f,
	0.00372644f, 0.00247454f, 0.00123242f, 0.f,
};

template <bool IsCubeTexture>
color_rgba32f sampler::sample_impl(int face, float coordx, float coordy, size_t sample, float miplevel, anisotropic_info const* af_info) const
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
		return sample_surface(*tex_->subresource_cptr(subres_index), coordx, coordy, sample, sampler_state_mag);
	}

	if(desc_.mip_filter == filter_point)
	{
		int ml = fast_floori(0.5 + miplevel);

        ml = eflib::clamp(ml, static_cast<int>(tex_->max_lod()), static_cast<int>(tex_->min_lod()));

		auto subres_index = compute_cube_subresource(dummy, face_sz, ml);
		return sample_surface(*tex_->subresource_cptr(subres_index), coordx, coordy, sample, sampler_state_min);
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

		color_rgba32f c0 = sample_surface(*tex_->subresource_cptr(subres_index_lo), coordx, coordy, sample, sampler_state_min);
		color_rgba32f c1 = sample_surface(*tex_->subresource_cptr(subres_index_hi), coordx, coordy, sample, sampler_state_min);

		return lerp(c0, c1, frac);
	}

	if(desc_.mip_filter == filter_anisotropic)
	{
		float start_relative_distance = - 0.5f * (af_info->probe_count - 1.0f);

		float sample_coord_x = coordx + af_info->delta_uv.x() * start_relative_distance;
		float sample_coord_y = coordy + af_info->delta_uv.y() * start_relative_distance;

		int lo = fast_roundi(miplevel);
		auto lo_sz = clamp(static_cast<size_t>(lo), tex_->max_lod(), tex_->min_lod());

		vec4 color(0.0f, 0.0f, 0.0f, 0.0f);
		float w_sum = 0.0f;
		int probe_count = static_cast<int>(af_info->probe_count);
		for(int i_sample = - probe_count + 1; i_sample < probe_count; i_sample += 2)
		{
			auto subres_index_lo = compute_cube_subresource(dummy, face_sz, lo_sz);
			color_rgba32f c0 = sample_surface(
				*tex_->subresource_cptr(subres_index_lo), sample_coord_x, sample_coord_y, sample, sampler_state_min
				);
			float w = EWA_WTS[static_cast<int>(i_sample * i_sample * af_info->weight_D)];

			color += c0.get_vec4() * w;
			w_sum += w;
			sample_coord_x += af_info->delta_uv.x();
			sample_coord_y += af_info->delta_uv.y();
		}

		color /= w_sum;

		return color_rgba32f(color);
	}

	EFLIB_ASSERT(false, "Mip filters is error.");
	return desc_.border_color;
}
#pragma optimize("", on)

color_rgba32f sampler::sample(float coordx, float coordy, float miplevel) const
{
	return sample_impl<false>(0, coordx, coordy, 0, miplevel, nullptr);
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

	return sample_impl<true>(major_dir, s, t, 0, miplevel, nullptr);
}

float sampler::calc_lod_2d(eflib::vec2 const& ddx, eflib::vec2 const& ddy) const
{
	uint4 size = tex_->size();

	vec4 ddx_vec4(ddx[0], ddx[1], 0.0f, 0.0f);
	vec4 ddy_vec4(ddy[0], ddy[1], 0.0f, 0.0f);

	float lod{0.0f};

	anisotropic_info af_info;
	if( desc_.mip_filter == filter_anisotropic && desc_.max_anisotropy > 1 )
	{
		calc_anisotropic_info(size, ddx_vec4, ddy_vec4, 0.0f, af_info);
		lod = af_info.lod;
	}
	else
	{
		lod = calc_lod(size, ddx_vec4, ddy_vec4, 0.0f);
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

	anisotropic_info af_info;
	float lod{ 0.0f };
	if( desc_.mip_filter == filter_anisotropic && desc_.max_anisotropy > 1 )
	{
		calc_anisotropic_info(size, ddx_vec4, ddy_vec4, lod_bias, af_info);
		lod = af_info.lod;
	}
	else
	{
		lod = calc_lod(size, ddx_vec4, ddy_vec4, lod_bias);
	}

	return sample_impl<false>(0, proj_coord[0], proj_coord[1], 0, lod, &af_info);
}

void sampler::calc_anisotropic_info(
	eflib::uint4 const& size,
	eflib::vec4 const& ddx, eflib::vec4 const& ddy, float bias,
	anisotropic_info& out_af_info ) const
{
	static int i = 0;
	vec4 size_vec4( static_cast<float>(size[0]), static_cast<float>(size[1]), static_cast<float>(size[2]), 0 );

	vec4 ddx_ts = ddx * size_vec4;
	vec4 ddy_ts = ddy * size_vec4;

	float ddx_len = ddx_ts.xy().length();
	float ddy_len = ddy_ts.xy().length();

	float diag0_len = (ddx_ts - ddy_ts).xy().length();
	float diag1_len = (ddx_ts + ddy_ts).xy().length();

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

#if 0
	// EWA axes computation for reference and debugging.
	float A_nn = ddx_ts.y() * ddx_ts.y() + ddy_ts.y() * ddy_ts.y();
	float B_nn = -2.0f * (ddx_ts.x() * ddx_ts.y() + ddy_ts.x() * ddy_ts.y());
	float C_nn = ddx_ts.x() * ddx_ts.x() + ddy_ts.x() * ddy_ts.x();
	float F = A_nn * C_nn - B_nn * B_nn * 0.25f;

	float A = A_nn / F;
	float B = B_nn / F;
	float C = C_nn / F;

	float root = sqrtf((A - C) * (A - C) + B * B);
	float A_prime = (A + C - root) * 0.5f;
	float C_prime = (A + C + root) * 0.5f;

	float ewa_major = sqrtf(1.0f / A_prime);
	float ewa_minor = sqrtf(1.0f / C_prime);

	if (i < 32)
	{
		printf(
			"%7.4f,%7.4f;%7.4f,%7.4f;%7.4f,%7.4f,%7.4f,%7.4f\n",
			ddx.x(), ddx.y(), ddy.x(), ddy.y(),
			minor_axis_len, long_axis_len, ewa_minor, ewa_major
		);
		++i;
	}
#endif

	float probe_count = (2.0f * long_axis_len / minor_axis_len) - 1.0f;
	float rounded_probe_count = fast_round(probe_count);
	rounded_probe_count = std::min(static_cast<float>(desc_.max_anisotropy), rounded_probe_count);

	if (rounded_probe_count < probe_count)
	{
		minor_axis_len = 2.0f * long_axis_len / (rounded_probe_count + 1.0f);
	}

	out_af_info.lod = fast_log2(minor_axis_len) + bias;
	out_af_info.probe_count = rounded_probe_count;
	
	if (rounded_probe_count <= 1.0f)
	{
		out_af_info.delta_uv = vec4::zero();
		out_af_info.weight_D = 0.0f;
	}
	else
	{
		float r = minor_axis_len / long_axis_len;
		out_af_info.delta_uv = (*long_axis) * (1.0f - r) * 2.0f * (1.0f / (rounded_probe_count - 1.0f));
		out_af_info.weight_D = static_cast<float>(EWA_MAXIDX + 1) * out_af_info.delta_uv.length_sqr() * 0.25f / (long_axis_len * long_axis_len);
		out_af_info.delta_uv.x() /= size_vec4.x();
		out_af_info.delta_uv.y() /= size_vec4.y();
	}
}

}
