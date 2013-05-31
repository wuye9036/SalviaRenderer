#include <salviar/include/rasterizer.h>

#include <salviar/include/clipper.h>
#include <salviar/include/framebuffer.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/shader_reflection.h>
#include <salviar/include/shader_object.h>
#include <salviar/include/shader_unit.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shaderregs_op.h>
#include <salviar/include/thread_pool.h>
#include <salviar/include/vertex_cache.h>

#include <eflib/include/diagnostics/log.h>
#include <eflib/include/platform/cpuinfo.h>
#include <eflib/include/platform/ext_intrinsics.h>
#include <eflib/include/utility/unref_declarator.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

using eflib::num_available_threads;

using boost::atomic;

class shader_reflection;

BEGIN_NS_SALVIAR();

using namespace std;
using namespace eflib;
using namespace boost;

const int TILE_SIZE = 64;
const int GEOMETRY_SETUP_PACKAGE_SIZE = 8;
const int DISPATCH_PRIMITIVE_PACKAGE_SIZE = 8;
const int RASTERIZE_PRIMITIVE_PACKAGE_SIZE = 1;
const int COMPACT_CLIPPED_VERTS_PACKAGE_SIZE = 8;

/*************************************************
 *   Steps for line rasterization£º
 *			1 Find major direction and computing distance and differential on major direction.
 *			2 Calculate ddx and ddy for mip-mapping.
 *			3 Computing pixel position and interpolated attribute by DDA with major direction and differential.
 *			4 Executing pixel shader
 *			5 Render pixel into framebuffer.
 *
 *   Note: 
 *			1 Position is in window coordinate.
 *			2 x y z components of w-pos have been divided by w component.
 *			3 positon.w() = 1.0f / clip w
 **************************************************/
void rasterizer::rasterize_line(
	uint32_t /*prim_id*/, const vs_output& v0, const vs_output& v1, const viewport& vp, const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu)
{
	EFLIB_UNREF_DECLARATOR( psu );

	const vs_output_op* vs_output_ops = pparent_->get_vs_output_ops();

	vs_output diff;
	vs_output_ops->sub(diff, v1, v0);
	eflib::vec4 const& dir = diff.position();
	float diff_dir = abs(dir.x()) > abs(dir.y()) ? dir.x() : dir.y();

	h_blend_shader hbs = pparent_->get_blend_shader();

	// Computing differential.
	vs_output ddx, ddy;
	vs_output_ops->mul(ddx, diff, (diff.position().x() / (diff.position().xy().length_sqr())));
	vs_output_ops->mul(ddy, diff, (diff.position().y() / (diff.position().xy().length_sqr())));

	int vpleft = fast_floori(max(0.0f, vp.x));
	int vptop = fast_floori(max(0.0f, vp.y));
	int vpright = fast_floori(min(vp.x+vp.w, (float)(frame_buffer_->get_width())));
	int vpbottom = fast_floori(min(vp.y+vp.h, (float)(frame_buffer_->get_height())));

	ps_output px_out;

	// Divided drawing to x major DDA method and y major DDA method.
	if( abs(dir.x()) > abs(dir.y()))
	{

		//Swap start and end to make diff is positive.
		const vs_output *start, *end;
		if(dir.x() < 0){
			start = &v1;
			end = &v0;
			diff_dir = -diff_dir;
		} else {
			start = &v0;
			end = &v1;
		}

		triangle_info info;
		info.set(start->position(), ddx, ddy);
		pps->ptriangleinfo_ = &info;

		float fsx = fast_floor(start->position().x() + 0.5f);

		int sx = fast_floori(fsx);
		int ex = fast_floori(end->position().x() - 0.5f);

		// Clamp to visible screen.
		sx = eflib::clamp<int>(sx, vpleft, int(vpright - 1));
		ex = eflib::clamp<int>(ex, vpleft, int(vpright));

		// Set attributes of start point.
		vs_output px_start, px_end;
		vs_output_ops->copy(px_start, *start);
		vs_output_ops->copy(px_end, *end);
		float step = sx + 0.5f - start->position().x();
		vs_output px_in;
		vs_output_ops->lerp(px_in, px_start, px_end, step / diff_dir);

		// Draw line with x major DDA.
		vs_output unprojed;
		for(int iPixel = sx; iPixel < ex; ++iPixel)
		{
			// Ingore pixels which are outside of viewport.
			if(px_in.position().y() >= vpbottom){
				if(dir.y() > 0) break;
				continue;
			}
			if(px_in.position().y() < 0){
				if(dir.y() < 0) break;
				continue;
			}

			// Render pixel.
			vs_output_ops->unproject(unprojed, px_in);
			if(pps->execute(unprojed, px_out)){
				frame_buffer_->render_sample(hbs, iPixel, fast_floori(px_in.position().y()), 0, px_out, px_out.depth);
			}

			// Increment ddx
			++ step;
			vs_output_ops->lerp(px_in, px_start, px_end, step / diff_dir);
		}
	}
	else //y major
	{
		const vs_output *start, *end;
		if(dir.y() < 0){
			start = &v1;
			end = &v0;
			diff_dir = -diff_dir;
		} else {
			start = &v0;
			end = &v1;
		}

		triangle_info info;
		info.set(start->position(), ddx, ddy);
		pps->ptriangleinfo_ = &info;

		float fsy = fast_floor(start->position().y() + 0.5f);

		int sy = fast_floori(fsy);
		int ey = fast_floori(end->position().y() - 0.5f);

		sy = eflib::clamp<int>(sy, vptop, int(vpbottom - 1));
		ey = eflib::clamp<int>(ey, vptop, int(vpbottom));

		vs_output px_start, px_end;
		vs_output_ops->copy(px_start, *start);
		vs_output_ops->copy(px_end, *end);
		float step = sy + 0.5f - start->position().y();
		vs_output px_in;
		vs_output_ops->lerp(px_in, px_start, px_end, step / diff_dir);

		vs_output unprojed;
		for(int iPixel = sy; iPixel < ey; ++iPixel)
		{
			if(px_in.position().x() >= vpright){
				if(dir.x() > 0) break;
				continue;
			}
			if(px_in.position().x() < 0){
				if(dir.x() < 0) break;
				continue;
			}

			vs_output_ops->unproject(unprojed, px_in);
			if(pps->execute(unprojed, px_out)){
				frame_buffer_->render_sample(hbs, fast_floori(px_in.position().x()), iPixel, 0, px_out, px_out.depth);
			}

			++ step;
			vs_output_ops->lerp(px_in, px_start, px_end, step / diff_dir);
		}
	}
}

struct tile_render_context
{
	int					left, top, right, bottom;
	size_t				num_samples;
	vs_output const*	v0;
	vs_output const*	ddx;
	vs_output const*	ddy;
	vs_output_op const* vs_output_ops;
	pixel_shader*		cpp_ps;
	blend_shader*		cpp_bs;
	pixel_shader_unit*	ps;
	float const*		aa_z_offset;
};

void rasterizer::initialize(renderer_impl* pparent)
{
	pparent_ = pparent;
	frame_buffer_ = pparent->get_framebuffer();
}

void rasterizer::draw_whole_tile(
	int left, int top, int right, int bottom, 
	size_t num_samples, const vs_output& v0,
	const vs_output& ddx, const vs_output& ddy, const vs_output_op* vs_output_ops,
	const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu, const h_blend_shader& hbs,
	const float* aa_z_offset)
{
	float v0x = v0.position().x();
	float v0y = v0.position().y();

	const float offsetx = 0.5f - v0x;
	const float offsety = 0.5f - v0y;

	for(int iy = top; iy < bottom; iy += 4)
	{
		vs_output px_in;
		for(int ix = left; ix < right; ix += 4)
		{
			vs_output unprojed[4*4];

			for(int dy = 0; dy < 4; ++dy)
			{
				for(int dx = 0; dx < 4; ++dx)
				{
					vs_output_ops->step_2d_unproj(
						unprojed[dx+dy*4],
						v0,
						offsetx+ix+dx, ddx,
						offsety+iy+dy, ddy
						);
				}
			}

			draw_full_package(unprojed, iy, ix, num_samples, hbs, pps, psu, aa_z_offset);
		}
	}
}

void rasterizer::draw_pixels(
	int left0, int top0, int left, int top,
	const eflib::vec4* edge_factors, size_t num_samples, bool has_centroid,
	const vs_output& v0, const vs_output& ddx, const vs_output& ddy, const vs_output_op* vs_output_ops,
	const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu, const h_blend_shader& hbs,
	const float* aa_z_offset )
{
	size_t sx = left - left0;
	size_t sy = top - top0;
	const uint32_t full_mask = (1UL << num_samples) - 1;

	uint32_t pixel_mask[4 * 4];
	memset(pixel_mask, 0, sizeof(pixel_mask));

#ifndef EFLIB_NO_SIMD
	const __m128 mtx = _mm_set_ps(1, 0, 1, 0);
	const __m128 mty = _mm_set_ps(1, 1, 0, 0);

	__m128 medge0 = _mm_load_ps(&edge_factors[0].x());
	__m128 medge1 = _mm_load_ps(&edge_factors[1].x());
	__m128 medge2 = _mm_load_ps(&edge_factors[2].x());

	__m128 mtmp = _mm_unpacklo_ps(medge0, medge1);
	__m128 medgex = _mm_shuffle_ps(mtmp, medge2, _MM_SHUFFLE(3, 0, 1, 0));
	__m128 medgey = _mm_shuffle_ps(mtmp, medge2, _MM_SHUFFLE(3, 1, 3, 2));
	mtmp = _mm_unpackhi_ps(medge0, medge1);
	__m128 medgez = _mm_shuffle_ps(mtmp, medge2, _MM_SHUFFLE(3, 2, 1, 0));

	__m128 mleft = _mm_set1_ps(left);
	__m128 mtop = _mm_set1_ps(top);
	__m128 mevalue3 = _mm_sub_ps(medgez, _mm_add_ps(_mm_mul_ps(mleft, medgex), _mm_mul_ps(mtop, medgey)));

	for (size_t i_sample = 0; i_sample < num_samples; ++ i_sample){
		const vec2& sp = samples_pattern_[i_sample];
		__m128 mspx = _mm_set1_ps(sp.x());
		__m128 mspy = _mm_set1_ps(sp.y());

		for(int iy = 0; iy < 4; ++ iy){
			__m128 my = _mm_add_ps(mspy, _mm_set1_ps(iy));
			__m128 mx = _mm_add_ps(mspx, _mm_set_ps(3, 2, 1, 0));

			__m128 mask_rej = _mm_setzero_ps();
			{
				__m128 mstepx = _mm_shuffle_ps(medgex, medgex, _MM_SHUFFLE(0, 0, 0, 0));
				__m128 mstepy = _mm_shuffle_ps(medgey, medgey, _MM_SHUFFLE(0, 0, 0, 0));
				__m128 msteprej = _mm_add_ps(_mm_mul_ps(mx, mstepx), _mm_mul_ps(my, mstepy));

				__m128 mevalue = _mm_shuffle_ps(mevalue3, mevalue3, _MM_SHUFFLE(0, 0, 0, 0));

				mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(msteprej, mevalue));
			}
			{
				__m128 mstepx = _mm_shuffle_ps(medgex, medgex, _MM_SHUFFLE(1, 1, 1, 1));
				__m128 mstepy = _mm_shuffle_ps(medgey, medgey, _MM_SHUFFLE(1, 1, 1, 1));
				__m128 msteprej = _mm_add_ps(_mm_mul_ps(mx, mstepx), _mm_mul_ps(my, mstepy));

				__m128 mevalue = _mm_shuffle_ps(mevalue3, mevalue3, _MM_SHUFFLE(1, 1, 1, 1));

				mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(msteprej, mevalue));
			}
			{
				__m128 mstepx = _mm_shuffle_ps(medgex, medgex, _MM_SHUFFLE(2, 2, 2, 2));
				__m128 mstepy = _mm_shuffle_ps(medgey, medgey, _MM_SHUFFLE(2, 2, 2, 2));
				__m128 msteprej = _mm_add_ps(_mm_mul_ps(mx, mstepx), _mm_mul_ps(my, mstepy));

				__m128 mevalue = _mm_shuffle_ps(mevalue3, mevalue3, _MM_SHUFFLE(2, 2, 2, 2));

				mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(msteprej, mevalue));
			}

			__m128 sample_mask = eflib_mm_castsi128_ps(_mm_set1_epi32(1UL << i_sample));
			sample_mask = _mm_andnot_ps(mask_rej, sample_mask);

			ALIGN16 uint32_t store[4];
			_mm_store_ps(reinterpret_cast<float*>(store), sample_mask);
			pixel_mask[iy * 4 + 0] |= store[0];
			pixel_mask[iy * 4 + 1] |= store[1];
			pixel_mask[iy * 4 + 2] |= store[2];
			pixel_mask[iy * 4 + 3] |= store[3];
		}
	}
#else
	float evalue[3];
	for (int e = 0; e < 3; ++ e)
	{
		evalue[e] = edge_factors[e].z() - (left * edge_factors[e].x() + top * edge_factors[e].y());
	}

	for(int iy = 0; iy < 4; ++iy)
	{
		// Rasterizer.
		for(size_t ix = 0; ix < 4; ++ix)
		{
			for (int i_sample = 0; i_sample < num_samples; ++ i_sample)
			{
				vec2  const& sp = samples_pattern_[i_sample];
				float const  fx = ix + sp.x();
				float const  fy = iy + sp.y();
				bool inside = true;
				for (int e = 0; e < 3; ++ e)
				{
					if (fx * edge_factors[e].x() + fy * edge_factors[e].y() < evalue[e])
					{
						inside = false;
						break;
					}
				}
				
				if (inside)
				{
					pixel_mask[iy * 4 + ix] |= 1UL << i_sample;
				}
			}
		}
	}
#endif

	const float offsetx = left + 0.5f - v0.position().x();
	const float offsety = top  + 0.5f - v0.position().y();

	// Compute unprojected pixels.
	vs_output unprojed[4*4];
	float dx, dy;

	for( int iy = 0; iy < 4; ++iy )
	{
		dy = offsety + iy;
		for(int ix = 0; ix < 4; ++ix)
		{
			uint32_t mask = pixel_mask[iy*4+ix];
			dx = offsetx + ix;
			
			// if ( mask ){
			if (has_centroid && (mask != full_mask) && mask != 0)
			{
				// centroid interpolate
				vec2 sp_centroid(0, 0);
				int n = 0;
				unsigned long i_sample;
				uint32_t const mask_backup = mask;
				while (_BitScanForward(&i_sample, mask))
				{
					const vec2& sp = samples_pattern_[i_sample];
					sp_centroid.x() += sp.x() - 0.5f;
					sp_centroid.y() += sp.y() - 0.5f;
					++ n;

					mask &= mask - 1;
				}
				sp_centroid /= n;

				mask = mask_backup;

				dx += sp_centroid.x();
				dy += sp_centroid.y();
			}

			vs_output_ops->step_2d_unproj(unprojed[iy*4+ix], v0, dx, ddx, dy, ddy);
		}
	}

	// Execute pixel shader and render to target.
	draw_package(unprojed, top, left, num_samples, hbs, pps, psu, pixel_mask, aa_z_offset);
}

void rasterizer::subdivide_tile(int left, int top, const eflib::rect<uint32_t>& cur_region,
		const vec4* edge_factors, uint32_t* test_regions, uint32_t& test_region_size, float x_min, float x_max, float y_min, float y_max,
		const float* rej_to_acc, const float* evalue, const float* step_x, const float* step_y){
	const uint32_t new_w = cur_region.w;
	const uint32_t new_h = cur_region.h;

#ifndef EFLIB_NO_SIMD
	static const union
	{
		int maski;
		float maskf;
	} MASK = { 0x80000000 };
	static const __m128 SIGN_MASK = _mm_set1_ps(MASK.maskf);

	__m128 medge0 = _mm_load_ps(&edge_factors[0].x());
	__m128 medge1 = _mm_load_ps(&edge_factors[1].x());
	__m128 medge2 = _mm_load_ps(&edge_factors[2].x());

	__m128 mtmp = _mm_unpacklo_ps(medge0, medge1);
	__m128 medgex = _mm_shuffle_ps(mtmp, medge2, _MM_SHUFFLE(3, 0, 1, 0));
	__m128 medgey = _mm_shuffle_ps(mtmp, medge2, _MM_SHUFFLE(3, 1, 3, 2));
	mtmp = _mm_unpackhi_ps(medge0, medge1);

	__m128 mstepx3 = _mm_load_ps(step_x);
	__m128 mstepy3 = _mm_load_ps(step_y);
	__m128 mrej2acc3 = _mm_load_ps(rej_to_acc);

	__m128 mleft = _mm_set1_ps(left);
	__m128 mtop = _mm_set1_ps(top);
	__m128 mevalue3 = _mm_sub_ps(_mm_load_ps(evalue), _mm_add_ps(_mm_mul_ps(mleft, medgex), _mm_mul_ps(mtop, medgey)));

	for(int iy = 0; iy < 4; ++ iy){
		__m128 my = _mm_mul_ps(mstepy3, _mm_set1_ps(iy));

		__m128 mask_rej = _mm_setzero_ps();
		__m128 mask_acc = _mm_setzero_ps();

		// Trival rejection & acception
		{
			__m128 mstepx = _mm_mul_ps(_mm_shuffle_ps(mstepx3, mstepx3, _MM_SHUFFLE(0, 0, 0, 0)), _mm_set_ps(3, 2, 1, 0));
			__m128 mstepy = _mm_shuffle_ps(my, my, _MM_SHUFFLE(0, 0, 0, 0));

			__m128 mrej2acc = _mm_shuffle_ps(mrej2acc3, mrej2acc3, _MM_SHUFFLE(0, 0, 0, 0));

			__m128 msteprej = _mm_add_ps(mstepx, mstepy);
			__m128 mstepacc = _mm_add_ps(msteprej, mrej2acc);

			__m128 mevalue = _mm_shuffle_ps(mevalue3, mevalue3, _MM_SHUFFLE(0, 0, 0, 0));

			mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(msteprej, mevalue));
			mask_acc = _mm_or_ps(mask_acc, _mm_cmplt_ps(mstepacc, mevalue));
		}
		{
			__m128 mstepx = _mm_mul_ps(_mm_shuffle_ps(mstepx3, mstepx3, _MM_SHUFFLE(1, 1, 1, 1)), _mm_set_ps(3, 2, 1, 0));
			__m128 mstepy = _mm_shuffle_ps(my, my, _MM_SHUFFLE(1, 1, 1, 1));

			__m128 mrej2acc = _mm_shuffle_ps(mrej2acc3, mrej2acc3, _MM_SHUFFLE(1, 1, 1, 1));

			__m128 msteprej = _mm_add_ps(mstepx, mstepy);
			__m128 mstepacc = _mm_add_ps(msteprej, mrej2acc);

			__m128 mevalue = _mm_shuffle_ps(mevalue3, mevalue3, _MM_SHUFFLE(1, 1, 1, 1));

			mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(msteprej, mevalue));
			mask_acc = _mm_or_ps(mask_acc, _mm_cmplt_ps(mstepacc, mevalue));
		}
		{
			__m128 mstepx = _mm_mul_ps(_mm_shuffle_ps(mstepx3, mstepx3, _MM_SHUFFLE(2, 2, 2, 2)), _mm_set_ps(3, 2, 1, 0));
			__m128 mstepy = _mm_shuffle_ps(my, my, _MM_SHUFFLE(2, 2, 2, 2));

			__m128 mrej2acc = _mm_shuffle_ps(mrej2acc3, mrej2acc3, _MM_SHUFFLE(2, 2, 2, 2));

			__m128 msteprej = _mm_add_ps(mstepx, mstepy);
			__m128 mstepacc = _mm_add_ps(msteprej, mrej2acc);

			__m128 mevalue = _mm_shuffle_ps(mevalue3, mevalue3, _MM_SHUFFLE(2, 2, 2, 2));

			mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(msteprej, mevalue));
			mask_acc = _mm_or_ps(mask_acc, _mm_cmplt_ps(mstepacc, mevalue));
		}
		mask_acc = _mm_andnot_ps(mask_acc, SIGN_MASK);

		__m128i mineww = _mm_set1_epi32(new_w);
		__m128i minewh = _mm_set1_epi32(new_h);
		__m128i mix = _mm_set_epi32(3 * new_w, 2 * new_w, 1 * new_w, 0 * new_h);
		__m128i miy = _mm_set1_epi32(iy * new_h);
		mix = _mm_add_epi32(mix, _mm_set1_epi32(cur_region.x));
		miy = _mm_add_epi32(miy, _mm_set1_epi32(cur_region.y));
		__m128i miregion = _mm_or_si128(mix, _mm_slli_epi32(miy, 8));
		miregion = _mm_or_si128(miregion, eflib_mm_castps_si128(mask_acc));

		ALIGN16 uint32_t region_code[4];
		_mm_store_si128(reinterpret_cast<__m128i*>(&region_code[0]), miregion);

		mask_rej = _mm_or_ps(mask_rej, _mm_cmpge_ps(_mm_set1_ps(x_min), _mm_cvtepi32_ps(_mm_add_epi32(mix, mineww))));
		mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(_mm_set1_ps(x_max), _mm_cvtepi32_ps(mix)));
		mask_rej = _mm_or_ps(mask_rej, _mm_cmpge_ps(_mm_set1_ps(y_min), _mm_cvtepi32_ps(_mm_add_epi32(miy, minewh))));
		mask_rej = _mm_or_ps(mask_rej, _mm_cmplt_ps(_mm_set1_ps(y_max), _mm_cvtepi32_ps(miy)));

		int rejections = ~_mm_movemask_ps(mask_rej) & 0xF;
		unsigned long t;
		while (_BitScanForward(&t, rejections)){
			assert(t < 4);

			test_regions[test_region_size] = region_code[t];
			++ test_region_size;

			rejections &= rejections - 1;
		}
	}
#else
	float evalue1[3];
	for (int e = 0; e < 3; ++ e){
		evalue1[e] = evalue[e] - (left * edge_factors[e].x() + top * edge_factors[e].y());
	}

	for (int ty = 0; ty < 4; ++ ty){
		uint32_t y = cur_region.y + new_h * ty;
		for (int tx = 0; tx < 4; ++ tx){
			uint32_t x = cur_region.x + new_w * tx;

			if ((x_min < x + new_w) && (x_max >= x)
				&& (y_min < y + new_h) && (y_max >= y))
			{
				int rejection = 0;
				int acception = 1;

				// Trivial rejection & acception
				for (int e = 0; e < 3; ++ e){
					float step = tx * step_x[e] + ty * step_y[e];
					rejection |= (step < evalue1[e]);
					acception &= (step + rej_to_acc[e] >= evalue1[e]);
				}

				if (!rejection){
					test_regions[test_region_size] = x + (y << 8) + (acception << 31);
					++ test_region_size;
				}
			}
		}
	}
#endif
}

/*************************************************
*   Steps of triangle rasterization£º
*			1 Generate scan line and compute derivation of scanlines
*			2 Rasterize scan line by rasterizer_scanline_impl 
*			3 Generate vs_output/ps_input for pixels.
*			4 Execute pixel shader
*			5 Render pixel to frame buffer.
*
*   Note: 
*			1 All of position pixel is in window coordinate system.
*			2 x, y, z components of wpos have been devided by 'clip w'.
*			3 positon.w() == 1.0f / 'clip w'
**************************************************/
void rasterizer::rasterize_triangle(
	uint32_t prim_id, uint32_t full,
	const vs_output& v0, const vs_output& v1, const vs_output& v2, const viewport& vp, 
	const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu )
{
	const size_t num_samples = frame_buffer_->get_num_samples();
	const vs_output_op* vs_output_ops = pparent_->get_vs_output_ops();

	// Pick the vertex which is nearby center of viewport
	// It will get more precision in interpolation.
	vs_output const* verts[3] = { &v0, &v1, &v2 };
	double dist_sqr[3] =
	{ 
		fabs( v0.position().x() ) + fabs( v0.position().y() ),
		fabs( v1.position().x() ) + fabs( v1.position().y() ),
		fabs( v2.position().x() ) + fabs( v2.position().y() )
	};

	int reordered_index[3];
	if(dist_sqr[0] < dist_sqr[1])
	{
		if(dist_sqr[0] < dist_sqr[2])
		{
			reordered_index[0] = 0;
		}
		else
		{
			reordered_index[0] = 2;
		}
	}
	else
	{
		if(dist_sqr[1] < dist_sqr[2])
		{
			reordered_index[0] = 1;
		}
		else
		{
			reordered_index[0] = 2;
		}
	}
	reordered_index[1] = ( reordered_index[0] + 1 ) % 3;
	reordered_index[2] = ( reordered_index[1] + 1 ) % 3;

	vs_output const* reordered_verts[3] = { verts[reordered_index[0]], verts[reordered_index[1]], verts[reordered_index[2]] };

	// Compute centroid.
	bool has_centroid = false;
	for(size_t i_attr = 0; i_attr < num_vs_output_attributes_; ++i_attr)
	{
		if (vs_output_ops->attribute_modifiers[i_attr] & vs_output::am_centroid)
		{
			has_centroid = true;
		}
	}

	// Compute edge factor.
	const ALIGN16 vec4 edge_factors[3] = {
		vec4(edge_factors_[prim_id * 3 + 0], 0),
		vec4(edge_factors_[prim_id * 3 + 1], 0),
		vec4(edge_factors_[prim_id * 3 + 2], 0)
	};

	const bool mark_x[3] = {
		edge_factors[0].x() > 0, edge_factors[1].x() > 0, edge_factors[2].x() > 0
	};
	const bool mark_y[3] = {
		edge_factors[0].y() > 0, edge_factors[1].y() > 0, edge_factors[2].y() > 0
	};
	
	enum TRI_VS_TILE {
		TVT_FULL,
		TVT_PARTIAL,
		TVT_EMPTY,
		TVT_PIXEL
	};

	// Compute difference along edge.
	vs_output e01, e02;
	vs_output_ops->sub(e01, *reordered_verts[1], *reordered_verts[0]);
	vs_output_ops->sub(e02, *reordered_verts[2], *reordered_verts[0]);

	// Compute area of triangle.
	float area = cross_prod2(e02.position().xy(), e01.position().xy());
	if(equal<float>(area, 0.0f)) return;
	float inv_area = 1.0f / area;

	/**********************************************************
	*  Compute difference of attributes.
	*********************************************************/
	vs_output ddx, ddy;
	{
		// ddx = (e02 * e01.position.y - e02.position.y * e01) * inv_area;
		// ddy = (e01 * e02.position.x - e01.position.x * e02) * inv_area;
		vs_output tmp0, tmp1, tmp2;
		vs_output_ops->mul(
			ddx,
			vs_output_ops->sub(
				tmp2,
				vs_output_ops->mul( tmp0, e02, e01.position().y() ),
				vs_output_ops->mul( tmp1, e01, e02.position().y() )
			), inv_area);
		vs_output_ops->mul(
			ddy,
			vs_output_ops->sub(
				tmp2,
				vs_output_ops->mul( tmp0, e01, e02.position().x() ),
				vs_output_ops->mul( tmp1, e02, e01.position().x() )
			), inv_area);
	}

	triangle_info info;
	info.set(reordered_verts[0]->position(), ddx, ddy);
	if( !psu ){
		pps->ptriangleinfo_ = &info;
	}
	
	const float x_min = min(reordered_verts[0]->position().x(), min(reordered_verts[1]->position().x(), reordered_verts[2]->position().x())) - vp.x;
	const float x_max = max(reordered_verts[0]->position().x(), max(reordered_verts[1]->position().x(), reordered_verts[2]->position().x())) - vp.x;
	const float y_min = min(reordered_verts[0]->position().y(), min(reordered_verts[1]->position().y(), reordered_verts[2]->position().y())) - vp.y;
	const float y_max = max(reordered_verts[0]->position().y(), max(reordered_verts[1]->position().y(), reordered_verts[2]->position().y())) - vp.y;

	/*************************************************
	*   Draw triangles with Larrabee algorithm .
	*************************************************/

	uint32_t test_regions[2][TILE_SIZE / 2 * TILE_SIZE / 2];
	uint32_t test_region_size[2] = { 0, 0 };
	test_regions[0][0] = (full << 31);
	test_region_size[0] = 1;
	int src_stage = 0;
	int dst_stage = !src_stage;

	const int vpleft0 = fast_floori(vp.x);
	const int vpright0 = fast_floori(vp.x + vp.w);
	const int vptop0 = fast_floori(vp.y);
	const int vpbottom0 = fast_floori(vp.y + vp.h);

	uint32_t subtile_w = fast_floori(vp.w);
	uint32_t subtile_h = fast_floori(vp.h);

	ALIGN16 float step_x[4];
	ALIGN16 float step_y[4];
	ALIGN16 float rej_to_acc[4];
	ALIGN16 float evalue[4];
	float part_evalue[4];
	for (int e = 0; e < 3; ++ e){
		step_x[e] = TILE_SIZE * edge_factors[e].x();
		step_y[e] = TILE_SIZE * edge_factors[e].y();
		rej_to_acc[e] = -abs(step_x[e]) - abs(step_y[e]);
		part_evalue[e] = mark_x[e] * TILE_SIZE * edge_factors[e].x() + mark_y[e] * TILE_SIZE * edge_factors[e].y();
		evalue[e] = edge_factors[e].z() - part_evalue[e];
	}
	step_x[3] = step_y[3] = 0;

	float aa_z_offset[MAX_NUM_MULTI_SAMPLES];
	if (num_samples > 1){
		for (unsigned long i_sample = 0; i_sample < num_samples; ++ i_sample){
			const vec2& sp = samples_pattern_[i_sample];
			aa_z_offset[i_sample] = (sp.x() - 0.5f) * ddx.position().z() + (sp.y() - 0.5f) * ddy.position().z();
		}
	}

	while (test_region_size[src_stage] > 0){
		test_region_size[dst_stage] = 0;
		
		subtile_w /= 4;
		subtile_h /= 4;

		for (int e = 0; e < 3; ++ e){
			step_x[e] *= 0.25f;
			step_y[e] *= 0.25f;
			rej_to_acc[e] *= 0.25f;
			part_evalue[e] *= 0.25f;
			evalue[e] = edge_factors[e].z() - part_evalue[e];
		}

		for (size_t ivp = 0; ivp < test_region_size[src_stage]; ++ ivp){
			const uint32_t packed_region = test_regions[src_stage][ivp];
			eflib::rect<uint32_t> cur_region(packed_region & 0xFF, (packed_region >> 8) & 0xFF,
				subtile_w, subtile_h);
			TRI_VS_TILE intersect = (packed_region >> 31) ? TVT_FULL : TVT_PARTIAL;

			const int vpleft = max(0U, static_cast<unsigned>(vpleft0 + cur_region.x) );
			const int vptop = max(0U, static_cast<unsigned>(vptop0 + cur_region.y) );
			const int vpright = min(vpleft0 + cur_region.x + cur_region.w * 4, static_cast<uint32_t>(frame_buffer_->get_width()));
			const int vpbottom = min(vptop0 + cur_region.y + cur_region.h * 4, static_cast<uint32_t>(frame_buffer_->get_height()));

			// For one pixel region
			if ((TVT_PARTIAL == intersect) && (cur_region.w <= 1) && (cur_region.h <= 1)){
				intersect = TVT_PIXEL;
			}

			switch (intersect)
			{
			case TVT_EMPTY:
				// Empty tile. Do nothing.
				break;

			case TVT_FULL:
				// The whole tile is inside a triangle.
				this->draw_whole_tile(vpleft, vptop, vpright, vpbottom, num_samples,
					*reordered_verts[0], ddx, ddy, vs_output_ops,
					pps, psu, blend_shader_, aa_z_offset);
				break;

			case TVT_PIXEL:
				// The tile is small enough for pixel level matching.
				this->draw_pixels(vpleft0, vptop0, vpleft, vptop, 
					edge_factors, num_samples,
					has_centroid, *reordered_verts[0], ddx, ddy, vs_output_ops,
					pps, psu, blend_shader_, aa_z_offset);
				break;

			default:
				// Only a part of the triangle is inside the tile. So subdivide the tile into small ones.
				this->subdivide_tile(
					vpleft, vptop,
					cur_region, edge_factors,
					test_regions[dst_stage], test_region_size[dst_stage],
					x_min, x_max, y_min, y_max,
					rej_to_acc, evalue, step_x, step_y);
				break;
			}
		}

		src_stage = (src_stage + 1) & 1;
		dst_stage = !src_stage;
	}
}

rasterizer::rasterizer()
{
	state_.reset(new raster_state(raster_desc()));
}
rasterizer::~rasterizer()
{
}

void rasterizer::set_state(const h_rasterizer_state& state)
{
	state_ = state;
}

const h_rasterizer_state& rasterizer::get_state() const
{
	return state_;
}

void rasterizer::geometry_setup_func(geometry_setup_context const* ctxt)
{
	int32_t const num_packages = (ctxt->num_primitive + ctxt->package_size - 1) / ctxt->package_size;

	vs_output_op const*		vs_output_ops	= pparent_->get_vs_output_ops();
	vertex_cache*			dvc				= pparent_->get_vertex_cache().get();
	viewport const&			vp				= pparent_->get_viewport();

	clip_context clip_ctxt;
	clip_ctxt.vert_pool	= ctxt->vso_pool;
	clip_ctxt.vso_ops	= vs_output_ops;
	clip_ctxt.cull		= state_->get_cull_func();
	clip_ctxt.prim		= prim_;

	clipper clp;
	clp.set_context(&clip_ctxt);

	clip_results clip_rslt;

	int32_t local_working_package = (*ctxt->working_package)++;
	
	while (local_working_package < num_packages)
	{
		const int32_t start = local_working_package * ctxt->package_size;
		const int32_t end = min(ctxt->num_primitive, start + ctxt->package_size);

		clip_rslt.clipped_verts = ctxt->clipped_vertices + start*9;
		uint32_t&	package_num_clipped_verts = ctxt->num_clipped_vertices[local_working_package];
		
		package_num_clipped_verts = 0;

		for (int32_t i = start; i < end; ++ i)
		{
			if (3 == prim_size_)
			{
				vs_output* pv[3] = {
					&dvc->fetch(i*3+0),
					&dvc->fetch(i*3+1),
					&dvc->fetch(i*3+2)
				};
				
				// grand band culling
				float const GRAND_BAND_SCALE = 1.2f;
				eflib::vec4 t0 = clampss(eflib::vec4(-pv[0]->position().x(), -pv[0]->position().y(), pv[0]->position().x(), pv[0]->position().y()) - pv[0]->position().w() * GRAND_BAND_SCALE, 0, 1);
				eflib::vec4 t1 = clampss(eflib::vec4(-pv[1]->position().x(), -pv[1]->position().y(), pv[1]->position().x(), pv[1]->position().y()) - pv[1]->position().w() * GRAND_BAND_SCALE, 0, 1);
				eflib::vec4 t2 = clampss(eflib::vec4(-pv[2]->position().x(), -pv[2]->position().y(), pv[2]->position().x(), pv[2]->position().y()) - pv[2]->position().w() * GRAND_BAND_SCALE, 0, 1);
				eflib::vec4 t = t0 * t1 * t2;

				if ((0 == t.x()) && (0 == t.y()) && (0 == t.z()) && (0 == t.w()))
				{
					clp.clip(pv, &clip_rslt);

					clip_rslt.clipped_verts += clip_rslt.num_clipped_verts;
					package_num_clipped_verts += clip_rslt.num_clipped_verts;
				}
			}
			else if (2 == prim_size_)
			{
				assert(false);
			}
		}

		local_working_package = (*ctxt->working_package)++;
	}
}

void rasterizer::dispatch_primitive_func(
	std::vector<std::vector<uint32_t> >& tiles,
	vs_output** clipped_verts,
	int32_t prim_count, uint32_t stride,
	atomic<int32_t>& working_package, int32_t package_size )
{

	const viewport& vp = pparent_->get_viewport();
	int num_tiles_x = static_cast<size_t>(vp.w + TILE_SIZE - 1) / TILE_SIZE;
	int num_tiles_y = static_cast<size_t>(vp.h + TILE_SIZE - 1) / TILE_SIZE;

	const int32_t num_packages = (prim_count + package_size - 1) / package_size;

	float x_min;
	float x_max;
	float y_min;
	float y_max;
	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages)
	{
		const int32_t start = local_working_package * package_size;
		const int32_t end = min(prim_count, start + package_size);
		for (int32_t i = start; i < end; ++ i)
		{
			const vec4* pv[3];
			for (size_t j = 0; j < stride; ++ j)
			{
				pv[j] = &( clipped_verts[i*stride+j]->position() );
			}

			if (3 == stride)
			{
				// x * (y1 - y0) - y * (x1 - x0) - (y1 * x0 - x1 * y0)
				vec3* edge_factors = &edge_factors_[i * 3];
				for (int e = 0; e < 3; ++ e)
				{
					const int se = e;
					const int ee = (e + 1) % 3;
					edge_factors[e].x() = pv[se]->y() - pv[ee]->y();
					edge_factors[e].y() = pv[ee]->x() - pv[se]->x();
					edge_factors[e].z() = pv[ee]->x() * pv[se]->y() - pv[ee]->y() * pv[se]->x();
				}
			}

			x_min = pv[0]->x();
			x_max = pv[0]->x();
			y_min = pv[0]->y();
			y_max = pv[0]->y();
			for (size_t j = 1; j < stride; ++ j){
				x_min = min(x_min, pv[j]->x());
				x_max = max(x_max, pv[j]->x());
				y_min = min(y_min, pv[j]->y());
				y_max = max(y_max, pv[j]->y());
			}

			const int sx = std::min(fast_floori(std::max(0.0f, x_min) / TILE_SIZE), num_tiles_x);
			const int sy = std::min(fast_floori(std::max(0.0f, y_min) / TILE_SIZE), num_tiles_y);
			const int ex = std::min(fast_ceili(std::max(0.0f, x_max) / TILE_SIZE) + 1, num_tiles_x);
			const int ey = std::min(fast_ceili(std::max(0.0f, y_max) / TILE_SIZE) + 1, num_tiles_y);
			if ((sx + 1 == ex) && (sy + 1 == ey)){
				// Small primitive
				tiles[sy * num_tiles_x + sx].push_back(i << 1);
			}
			else{
				if (3 == stride){
					vec3* edge_factors = &edge_factors_[i * 3];

					const bool mark_x[3] = {
						edge_factors[0].x() > 0, edge_factors[1].x() > 0, edge_factors[2].x() > 0
					};
					const bool mark_y[3] = {
						edge_factors[0].y() > 0, edge_factors[1].y() > 0, edge_factors[2].y() > 0
					};
					
					float step_x[3];
					float step_y[3];
					float rej_to_acc[3];
					for (int e = 0; e < 3; ++ e){
						step_x[e] = TILE_SIZE * edge_factors[e].x();
						step_y[e] = TILE_SIZE * edge_factors[e].y();
						rej_to_acc[e] = -abs(step_x[e]) - abs(step_y[e]);
					}

					for (int y = sy; y < ey; ++ y){
						for (int x = sx; x < ex; ++ x){
							int rejection = 0;
							int acception = 1;

							// Trival rejection & acception
							for (int e = 0; e < 3; ++ e){
								float evalue = edge_factors[e].z() - ((x + mark_x[e]) * TILE_SIZE * edge_factors[e].x() + (y + mark_y[e]) * TILE_SIZE * edge_factors[e].y());
								rejection |= (0 < evalue);
								acception &= (rej_to_acc[e] >= evalue);
							}

							if (!rejection){
								tiles[y * num_tiles_x + x].push_back((i << 1) | acception);
							}
						}
					}
				}
				else{
					for (int y = sy; y < ey; ++ y){
						for (int x = sx; x < ex; ++ x){
							tiles[y * num_tiles_x + x].push_back(i << 1);
						}
					}
				}
			}
		}

		local_working_package = working_package ++;
	}
}

void rasterizer::rasterize_primitive_func(
	std::vector<std::vector<std::vector<uint32_t> > >& thread_tiles, int num_tiles_x,
	vs_output** clipped_verts,
	const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu, 
	atomic<int32_t>& working_package, int32_t package_size )
{
	const int32_t num_tiles = static_cast<int32_t>(thread_tiles[0].size());
	const int32_t num_packages = (num_tiles + package_size - 1) / package_size;

	const viewport& vp = pparent_->get_viewport();

	viewport tile_vp;
	tile_vp.w = TILE_SIZE;
	tile_vp.h = TILE_SIZE;
	tile_vp.minz = vp.minz;
	tile_vp.maxz = vp.maxz;

	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages){
		const int32_t start = local_working_package * package_size;
		const int32_t end = min(num_tiles, start + package_size);
		for (int32_t i = start; i < end; ++ i){
			std::vector<uint32_t> prims;
			for (size_t j = 0; j < thread_tiles.size(); ++ j){
				prims.insert(prims.end(), thread_tiles[j][i].begin(), thread_tiles[j][i].end());
			}
			std::sort(prims.begin(), prims.end());

			int y = i / num_tiles_x;
			int x = i - y * num_tiles_x;
			tile_vp.x = static_cast<float>(x * TILE_SIZE);
			tile_vp.y = static_cast<float>(y * TILE_SIZE);

			rasterize_func_(this, clipped_verts, prims, tile_vp, pps, psu);
		}

		local_working_package = working_package ++;
	}
}

void rasterizer::rasterize_line_func(
	vs_output** clipped_verts,
	const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp,
	const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu )
{
	for (std::vector<uint32_t>::const_iterator iter = sorted_prims.begin(); iter != sorted_prims.end(); ++ iter){
		uint32_t iprim = *iter >> 1;
		this->rasterize_line(
			iprim, *clipped_verts[iprim * 2 + 0], *clipped_verts[iprim * 2 + 1],
			tile_vp, pps, psu);
	}
}

void rasterizer::rasterize_triangle_func(
	vs_output** clipped_verts, 
	const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp,
	const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu )
{
	for (std::vector<uint32_t>::const_iterator iter = sorted_prims.begin(); iter != sorted_prims.end(); ++ iter){
		uint32_t iprim = *iter >> 1;
		uint32_t full = *iter & 1;
		this->rasterize_triangle(
			iprim, full,
			*clipped_verts[iprim * 3 + 0],
			*clipped_verts[iprim * 3 + 1],
			*clipped_verts[iprim * 3 + 2],
			tile_vp, pps, psu);
	}
}

void rasterizer::compact_clipped_verts_func(
	vs_output** compacted, vs_output** sparse,
	uint32_t const* addresses, uint32_t const* num_clipped_verts,
	int32_t workset, atomic<int32_t>& working_package, int32_t package_size)
{
	int32_t const num_packages = (workset + package_size - 1) / package_size;
	
	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages)
	{
		const int32_t start = local_working_package * package_size;
		const int32_t end = min(workset, start + package_size);
		for (int32_t i = start; i < end; ++ i)
		{
			memcpy(compacted+addresses[i], sparse+GEOMETRY_SETUP_PACKAGE_SIZE*i*9, num_clipped_verts[i]*sizeof(vs_output*));
		}

		local_working_package = working_package ++;
	}
}

void rasterizer::update_prim_info()
{	
	bool is_tri = false;
	bool is_line = false;
	bool is_point = false;

	bool is_wireframe = false;
	bool is_solid = false;

	switch (state_->get_desc().fm)
	{
	case fill_solid:
		is_solid = true;
		break;
	case fill_wireframe:
		is_wireframe = true;
		break;
	}

	primitive_topology prim_topo = pparent_->get_primitive_topology();
	switch (prim_topo)
	{
	case primitive_point_list:
	case primitive_point_sprite:
		is_point = false;
		break;
	case primitive_line_list:
	case primitive_line_strip:
		is_line = false;
		break;
	case primitive_triangle_list:
	case primitive_triangle_fan:
	case primitive_triangle_strip:
		is_tri = true;
		break;
	}

	if (is_solid && is_tri)
	{
		prim_ = pt_solid_tri;
	}
	else if (is_wireframe && is_tri)
	{
		prim_ = pt_wireframe_tri;
	}
	else
	{
		prim_ = pt_none;
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	switch(prim_)
	{
	case pt_line:
	case pt_wireframe_tri:
		prim_size_ = 2;
		break;
	case pt_solid_tri:
		prim_size_ = 3;
		break;
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}

void rasterizer::draw(size_t prim_count){
	assert(pparent_);
	if(!pparent_) return;

	blend_shader_ = pparent_->get_blend_shader();

	const size_t num_samples = frame_buffer_->get_num_samples();
	switch (num_samples){
	case 1:
		samples_pattern_[0] = vec2(0.5f, 0.5f);
		break;

	case 2:
		samples_pattern_[0] = vec2(0.25f, 0.25f);
		samples_pattern_[1] = vec2(0.75f, 0.75f);
		break;

	case 4:
		samples_pattern_[0] = vec2(0.375f, 0.125f);
		samples_pattern_[1] = vec2(0.875f, 0.375f);
		samples_pattern_[2] = vec2(0.125f, 0.625f);
		samples_pattern_[3] = vec2(0.625f, 0.875f);
		break;

	default:
		break;
	}

	update_prim_info();

	// Select rasterize func
	switch(prim_)
	{
	case pt_line:
	case pt_wireframe_tri:
		rasterize_func_ = boost::mem_fn(&rasterizer::rasterize_line_func);
		break;
	case pt_solid_tri:
		rasterize_func_ = boost::mem_fn(&rasterizer::rasterize_triangle_func);
		break;
	default:
		EFLIB_ASSERT(false, "Primitive type is not correct.");
	}

	if( pparent_->get_vertex_shader() )
	{
		num_vs_output_attributes_ = pparent_->get_vertex_shader()->num_output_attributes();
	}
	else
	{
		num_vs_output_attributes_ = 0;
	}

	viewport const& vp = pparent_->get_viewport();
	int num_tiles_x = static_cast<size_t>(vp.w + TILE_SIZE - 1) / TILE_SIZE;
	int num_tiles_y = static_cast<size_t>(vp.h + TILE_SIZE - 1) / TILE_SIZE;

	atomic<int32_t> working_package(0);
	size_t			geom_package_size = static_cast<size_t>(GEOMETRY_SETUP_PACKAGE_SIZE);
	size_t			num_geom_package  = (prim_count+geom_package_size-1)/geom_package_size;

	size_t num_threads = num_available_threads();

	// Culling, Clipping, Geometry Setup
	typedef eflib::pool::preserved_pool<vs_output> vs_output_pool;

	boost::shared_array<vs_output_pool>	vso_pools			(new vs_output_pool	[num_threads]);
	boost::shared_array<vs_output*>		clipped_verts		(new vs_output*		[prim_count*9]);
	boost::shared_array<uint32_t>		num_clipped_verts	(new uint32_t		[num_geom_package]);
	vector<geometry_setup_context>		geom_setup_ctxts	(num_threads);

	for (size_t i = 0; i < num_threads; ++ i)
	{
		vso_pools[i].reserve(prim_count*4);

		geom_setup_ctxts[i].clipped_vertices	= &(clipped_verts[0]);
		geom_setup_ctxts[i].num_clipped_vertices= &(num_clipped_verts[0]);
		geom_setup_ctxts[i].vso_pool			= &(vso_pools[i]);
		geom_setup_ctxts[i].num_primitive		= prim_count;
		geom_setup_ctxts[i].working_package		= &working_package;
		geom_setup_ctxts[i].package_size		= GEOMETRY_SETUP_PACKAGE_SIZE;

		if(i != num_threads-1)
		{
			global_thread_pool().schedule(
				boost::bind(&rasterizer::geometry_setup_func, this, &(geom_setup_ctxts[i]) )
				);
		}
		else
		{
			geometry_setup_func( &(geom_setup_ctxts[i]) );
		}
	}
	
	global_thread_pool().wait();

	// Compact Clipped Vertexes
	boost::shared_array<uint32_t> addresses(new uint32_t[num_geom_package]);
	addresses[0] = 0;
	for (size_t i = 1; i < num_geom_package; ++ i)
	{
		addresses[i] = addresses[i-1] + num_clipped_verts[i-1];
	}
	uint32_t total_num_clipped_verts = addresses[num_geom_package-1] + num_clipped_verts[num_geom_package-1];

	boost::shared_array<vs_output*> compacted_verts(new vs_output*[total_num_clipped_verts]);
	working_package = 0;
	for (size_t i = 0; i < num_threads - 1; ++ i)
	{
		global_thread_pool().schedule(
			boost::bind(
			&rasterizer::compact_clipped_verts_func, this, &compacted_verts[0],
			&clipped_verts[0], &addresses[0],
			&num_clipped_verts[0],
			static_cast<int32_t>(num_geom_package),
			boost::ref(working_package),
			COMPACT_CLIPPED_VERTS_PACKAGE_SIZE)
			);
	}
	compact_clipped_verts_func(
		&compacted_verts[0],
		&clipped_verts[0], &addresses[0],
		&num_clipped_verts[0],
		static_cast<int32_t>(num_geom_package),
		boost::ref(working_package),
		COMPACT_CLIPPED_VERTS_PACKAGE_SIZE
		);

	global_thread_pool().wait();

	// Project and Transformed to Viewport
	viewport_and_project_transform(compacted_verts.get(), total_num_clipped_verts); 

	// Dispatch primitives into tiles' bucket
	std::vector<std::vector<std::vector<uint32_t> > > thread_tiles(num_threads);
	working_package = 0;
	edge_factors_.resize(total_num_clipped_verts / prim_size_ * 3);
	for (size_t i = 0; i < num_threads - 1; ++ i){
		thread_tiles[i].resize(num_tiles_x * num_tiles_y);
		global_thread_pool().schedule(boost::bind(&rasterizer::dispatch_primitive_func, this, boost::ref(thread_tiles[i]),
			&compacted_verts[0], static_cast<int32_t>(total_num_clipped_verts / prim_size_),
			prim_size_, boost::ref(working_package), DISPATCH_PRIMITIVE_PACKAGE_SIZE));
	}
	thread_tiles[num_threads - 1].resize(num_tiles_x * num_tiles_y);
	dispatch_primitive_func(boost::ref(thread_tiles[num_threads - 1]),
		&compacted_verts[0], static_cast<int32_t>(total_num_clipped_verts / prim_size_),
		prim_size_, boost::ref(working_package), DISPATCH_PRIMITIVE_PACKAGE_SIZE);
	global_thread_pool().wait();

	// Rasterize tiles
	working_package = 0;
	h_pixel_shader hps = pparent_->get_pixel_shader();
	boost::shared_ptr<pixel_shader_unit> psu = pparent_->ps_proto();

	std::vector<h_pixel_shader> ppps(num_threads - 1);
	std::vector< boost::shared_ptr<pixel_shader_unit> > ppsu(num_threads-1);
	for (size_t i = 0; i < num_threads - 1; ++ i){
		// create pixel_shader clone per thread from hps
		if( hps ){
			ppps[i] = hps->create_clone();
		} 
		if( psu ) {
			ppsu[i] = psu->clone();
		}
		global_thread_pool().schedule(boost::bind(&rasterizer::rasterize_primitive_func, this, boost::ref(thread_tiles),
			num_tiles_x, &compacted_verts[0], ppps[i], ppsu[i], boost::ref(working_package), RASTERIZE_PRIMITIVE_PACKAGE_SIZE));
	}
	rasterize_primitive_func(boost::ref(thread_tiles), num_tiles_x, &compacted_verts[0], hps, psu, boost::ref(working_package), RASTERIZE_PRIMITIVE_PACKAGE_SIZE);
	global_thread_pool().wait();

	// destroy all pixel_shader clone
	for (size_t i = 0; i < num_threads - 1; ++ i){
		if( hps ){
			hps->destroy_clone(ppps[i]);
		}
	}
}

void rasterizer::draw_package(
	vs_output* pixels,
	uint32_t top, uint32_t left, size_t num_samples,
	h_blend_shader const& bs, h_pixel_shader const& pps, boost::shared_ptr<pixel_shader_unit> const& psu,
	uint32_t const* masks, float const* aa_z_offset )
{
	uint32_t const full_mask = (1UL << num_samples) - 1;

	shader_reflection const* vs_abi = pparent_->vs_proto() ? pparent_->vs_proto()->code->get_reflection() : NULL;

	ps_output pso[PACKAGE_ELEMENT_COUNT];
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i )
	{
		pso[i].depth = pixels[i].position().z();
		pso[i].front_face = pixels[i].front_face();
		pso[i].coverage = 0xFFFFFFFF;
	}

	if(psu)
	{
		psu->update( pixels, vs_abi );
		psu->execute( pso );
	}	

	for( int iy = 0; iy < 4; ++iy )
	{
		for ( int ix = 0; ix < 4; ++ix )
		{
			int px_index = iy * 4 + ix;
			uint32_t mask = masks[px_index];

			// No sampler need to be draw.
			if(!mask) continue;

			// Clipped by pixel shader.
			if( !psu && !pps->execute(pixels[px_index], pso[px_index]) ) continue;

			const int x_coord = left + ix;
			const int y_coord = top + iy;

			// Whole pixel
			if (1 == num_samples){
				frame_buffer_->render_sample(bs, x_coord, y_coord, 0, pso[px_index], pso[px_index].depth);
				continue;
			}

			mask &= pso[px_index].coverage;

			// MSAA.
			if (full_mask == mask)
			{
				for (unsigned long i_sample = 0; i_sample < num_samples; ++ i_sample)
				{
					frame_buffer_->render_sample(
						bs, x_coord, y_coord, i_sample,
						pso[px_index], pso[px_index].depth + aa_z_offset[i_sample]
					);
				}
			} 
			else
			{
				unsigned long i_sample;
				while (_BitScanForward(&i_sample, mask))
				{
					frame_buffer_->render_sample(
						bs, x_coord, y_coord, i_sample,
						pso[px_index], pso[px_index].depth + aa_z_offset[i_sample]
					);
					mask &= mask - 1;
				}
			}
		}
	}
}

void rasterizer::draw_full_package(
	vs_output* pixels,
	uint32_t top, uint32_t left, size_t num_samples,
	h_blend_shader const& bs, h_pixel_shader const& pps, boost::shared_ptr<pixel_shader_unit> const& psu,
	float const* aa_z_offset )
{
	uint32_t const full_mask = (1UL << num_samples) - 1;

	shader_reflection const* vs_abi = NULL;
	if( pparent_->vs_proto() ){
		vs_abi = pparent_->vs_proto()->code->get_reflection();
	}
	ps_output pso[PACKAGE_ELEMENT_COUNT];
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i )
	{
		pso[i].depth = pixels[i].position().z();
		pso[i].front_face = pixels[i].front_face();
		pso[i].coverage = 0xFFFFFFFF;
	}

	if( psu ){
		psu->update( pixels, vs_abi );
		psu->execute( pso );
	}

	for ( int iy = 0; iy < 4; ++iy ){
		for( int ix = 0; ix < 4; ++ix ){

			int const px_index = ix + iy * 4;

			if( !psu && !pps->execute(pixels[px_index], pso[px_index]) )
			{
				continue;
			}

			const int x_coord = left + ix;
			const int y_coord = top + iy;
			if (1 == num_samples)
			{
				frame_buffer_->render_sample(
					bs, x_coord, y_coord, 0, pso[px_index], pso[px_index].depth);
				continue;
			}

			uint32_t mask = pso[px_index].coverage;
			
			if (full_mask == mask)
			{
				for (unsigned long i_sample = 0; i_sample < num_samples; ++ i_sample)
				{
					frame_buffer_->render_sample(
						bs, x_coord, y_coord, i_sample,
						pso[px_index], pso[px_index].depth + aa_z_offset[i_sample]
					);
				}
			}
			else
			{
				unsigned long i_sample;
				while (_BitScanForward(&i_sample, mask))
				{
					frame_buffer_->render_sample(
						bs, x_coord, y_coord, i_sample,
						pso[px_index], pso[px_index].depth + aa_z_offset[i_sample]
					);
					mask &= mask - 1;
				}
			}
		}
	}

}


void rasterizer::viewport_and_project_transform(vs_output** vertexes, size_t num_verts)
{
	vs_output_functions::project proj_fn = pparent_->get_vs_output_ops()->project;
	viewport const& vp = pparent_->get_viewport();

	// Gathering vs_output need to be processed.
	vector<vs_output*> sorted;
	sorted.insert( sorted.end(), vertexes, vertexes+num_verts );
	std::sort( sorted.begin(), sorted.end() );
	sorted.erase( std::unique(sorted.begin(), sorted.end()), sorted.end() );

	// Transform vertex
	for(vector<vs_output*>::iterator iter = sorted.begin(); iter != sorted.end(); ++iter)
	{
		vs_output* vso = *iter;
		viewport_transform(vso->position(), vp);
		proj_fn(*vso, *vso);
	}
}

END_NS_SALVIAR();
