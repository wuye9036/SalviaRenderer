#include <salviar/include/shader_regs.h>
#include <salviar/include/shader_regs_op.h>
#include <salviar/include/shader.h>
#include <salviar/include/renderer.h>

#include <eflib/include/platform/intrin.h>

BEGIN_NS_SALVIAR();

using namespace boost;
using namespace eflib;

using std::make_pair;

#if defined(EFLIB_CPU_X86) || defined(EFLIB_CPU_X64) && !defined(EFLIB_NO_SIMD)
#define VSO_INTERP_SSE_ENABLED
#endif

template <int N> vs_input_op  gen_vs_input_op_n();
template <int N> vs_output_op gen_vs_output_op_n();

vs_input_op vs_input_ops[MAX_VS_INPUT_ATTRS] = {
	gen_vs_input_op_n<0>(),
	gen_vs_input_op_n<1>(),
	gen_vs_input_op_n<2>(),
	gen_vs_input_op_n<3>(),
	gen_vs_input_op_n<4>(),
	gen_vs_input_op_n<5>(),
	gen_vs_input_op_n<6>(),
	gen_vs_input_op_n<7>()
	//gen_vs_input_op_n<8>(),
	//gen_vs_input_op_n<9>(),
	//gen_vs_input_op_n<10>(),
	//gen_vs_input_op_n<11>(),
	//gen_vs_input_op_n<12>(),
	//gen_vs_input_op_n<13>(),
	//gen_vs_input_op_n<14>(),
	//gen_vs_input_op_n<15>()
};

vs_output_op vs_output_ops[MAX_VS_OUTPUT_ATTRS] = {
	gen_vs_output_op_n<0>(),
	gen_vs_output_op_n<1>(),
	gen_vs_output_op_n<2>(),
	gen_vs_output_op_n<3>(),
	gen_vs_output_op_n<4>(),
	gen_vs_output_op_n<5>()
	//gen_vs_output_op_n<6>(),
	//gen_vs_output_op_n<7>(),
	//gen_vs_output_op_n<8>(),
	//gen_vs_output_op_n<9>(),
	//gen_vs_output_op_n<10>(),
	//gen_vs_output_op_n<11>(),
	//gen_vs_output_op_n<12>(),
	//gen_vs_output_op_n<13>(),
	//gen_vs_output_op_n<14>(),
	//gen_vs_output_op_n<15>(),
	//gen_vs_output_op_n<16>(),
	//gen_vs_output_op_n<17>(),
	//gen_vs_output_op_n<18>(),
	//gen_vs_output_op_n<19>(),
	//gen_vs_output_op_n<20>(),
	//gen_vs_output_op_n<21>(),
	//gen_vs_output_op_n<22>(),
	//gen_vs_output_op_n<23>(),
	//gen_vs_output_op_n<24>(),
	//gen_vs_output_op_n<25>(),
	//gen_vs_output_op_n<26>(),
	//gen_vs_output_op_n<27>(),
	//gen_vs_output_op_n<28>(),
	//gen_vs_output_op_n<29>(),
	//gen_vs_output_op_n<30>(),
	//gen_vs_output_op_n<31>()
};

namespace vs_input_op_funcs
{
	template <int N>
	vs_input& construct_n(vs_input& out, eflib::vec4 const* attrs)
	{
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			out.attribute(i_attr) = attrs[i_attr];
		}
		return out;
	}
	template <int N>
	vs_input& copy_n(vs_input& out, const vs_input& in)
	{
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			out.attribute(i_attr) = in.attribute(i_attr);
		}
		return out;
	}
}

namespace vs_output_op_funcs
{
	template <int N>
	vs_output& construct_n(vs_output& out,
			const eflib::vec4& position,
			eflib::vec4 const* attribs)
	{
		out.position() = position;
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			out.attribute(i_attr) = attribs[i_attr];
		}
		return out;
	}
	template <int N>
	vs_output& copy_n(vs_output& out, const vs_output& in)
	{
		for(size_t i_register = 0; i_register < N+1; ++i_register)
		{
			__m128* dst = reinterpret_cast<__m128*>( out.raw_data() + i_register );
			__m128 const* src = reinterpret_cast<__m128 const*>( in.raw_data() + i_register );
			*dst = *src;
		}
		return out;
	}

	template <int N>
	vs_output& project_n(vs_output& out, const vs_output& in)
	{
		if (&out != &in){
			for(size_t i_attr = 0; i_attr < N; ++i_attr){
				out.attribute(i_attr) = in.attribute(i_attr);
				if (!(vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_noperspective)){
					out.attribute(i_attr) *= in.position().w();
				}
			}
			out.position() = in.position();
		}
		else{
			for(size_t i_attr = 0; i_attr < N; ++i_attr){
				if (!(vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_noperspective)){
					out.attribute(i_attr) *= in.position().w();
				}
			}
		}
		return out;
	}

	template <int N>
	vs_output& unproject_n(vs_output& out, const vs_output& in)
	{
		const float inv_w = 1.0f / in.position().w();
#if defined(VSO_INTERP_SSE_ENABLED)
		__m128*			dst = NULL;
		__m128 const*	src = NULL;

		__m128 inv_w4 = _mm_load_ps1(&inv_w);

		out.position() = in.position();
		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			src = reinterpret_cast<__m128 const*>(&in.attribute(i_attr));
			dst = reinterpret_cast<__m128*>(&out.attribute(i_attr));

			if (vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_noperspective)
			{
				*dst = *src;
			}
			else
			{
				*dst = _mm_mul_ps(*src, inv_w4);
			}
		}
#else
		out.position() = in.position();
		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			if (vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_noperspective)
			{
				out.attribute(i_attr) = in.attribute(i_attr);
			}
			else
			{
				out.attribute(i_attr) = in.attribute(i_attr) * inv_w;
			}
		}
#endif
		
		return out;
	}

	template <int N>
	vs_output& lerp_n(vs_output& out, const vs_output& start, const vs_output& end, float step)
	{
		out.position() = start.position() + ( end.position() - start.position() ) * step;
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			out.attribute(i_attr) = start.attribute(i_attr);
			if (!(vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
				out.attribute(i_attr) += (end.attribute(i_attr) - start.attribute(i_attr)) * step;
			}
		}
		return out;
	}

	vs_output& step_2d_unproj_pos(
		vs_output& out, const vs_output& in,
		float step0, const vs_output& derivation0,
		float step1, const vs_output& derivation1
        )
	{
#if defined(VSO_INTERP_SSE_ENABLED)
		__m128 const* d0_m128	= reinterpret_cast<__m128 const*>( derivation0.raw_data() );
		__m128 const* d1_m128	= reinterpret_cast<__m128 const*>( derivation1.raw_data() );
		__m128 const* in_m128	= reinterpret_cast<__m128 const*>( in.raw_data() );
		__m128*		  out_m128	= reinterpret_cast<__m128 *>( out.raw_data() );
		__m128		  step0_m128= _mm_load_ps1(&step0);
		__m128		  step1_m128= _mm_load_ps1(&step1);

		out_m128[0] = _mm_add_ps(
			in_m128[0],
			_mm_add_ps( _mm_mul_ps(d0_m128[0], step0_m128), _mm_mul_ps(d1_m128[0], step1_m128) )
			);

#else
		out.position() =
			in.position()
			+ (derivation0.position() * step0)
			+ (derivation1.position() * step1);

#endif
		return out;
	}

    template <int N>
	vs_output& step_2d_unproj_attr_n(
		vs_output& out, const vs_output& in,
		float step0, const vs_output& derivation0,
		float step1, const vs_output& derivation1
        )
	{
#if defined(VSO_INTERP_SSE_ENABLED)
		__m128 const* d0_m128	= reinterpret_cast<__m128 const*>( derivation0.raw_data() );
		__m128 const* d1_m128	= reinterpret_cast<__m128 const*>( derivation1.raw_data() );
		__m128 const* in_m128	= reinterpret_cast<__m128 const*>( in.raw_data() );
		__m128*		  out_m128	= reinterpret_cast<__m128 *>( out.raw_data() );
		__m128		  step0_m128= _mm_load_ps1(&step0);
		__m128		  step1_m128= _mm_load_ps1(&step1);

		float inv_w = 1.0f / _xmm_extract_ps(out_m128[0], 3);
		__m128 inv_w4 = _mm_load_ps1(&inv_w);

		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			__m128 interp_attr;
			if (vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_nointerpolation)
			{
				interp_attr = in_m128[i_attr+1];
			}
			else
			{
				interp_attr = _mm_add_ps(
					in_m128[i_attr + 1],
					_mm_add_ps(
						_mm_mul_ps(d0_m128[i_attr + 1], step0_m128),
						_mm_mul_ps(d1_m128[i_attr + 1], step1_m128)
						)
					);

			}

			// Perspective
			if (vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_noperspective)
			{
				out_m128[i_attr+1] = interp_attr;
			}
			else
			{
				out_m128[i_attr+1] = _mm_mul_ps(interp_attr, inv_w4);
			}
		}
#else
		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			if (vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_nointerpolation)
			{
				out.attribute(i_attr) = in.attribute(i_attr);
			}
			else
			{
				out.attribute(i_attr) =
					in.attribute(i_attr)
					+ (derivation0.attribute(i_attr) * step0)
					+ (derivation1.attribute(i_attr) * step1);
			}
		}
#endif

		return out;
	}

	vs_output& step_2d_unproj_pos_quad(
		vs_output* out, const vs_output& in,
		float step0, const vs_output& derivation0,
		float step1, const vs_output& derivation1
        )
	{
#if defined(VSO_INTERP_SSE_ENABLED)
		__m128  d0_m128 = _mm_load_ps( reinterpret_cast<float const*>( derivation0.raw_data() ) );
		__m128  d1_m128 = _mm_load_ps( reinterpret_cast<float const*>( derivation1.raw_data() ) );

		__m128  in_m128 = _mm_load_ps( reinterpret_cast<float const*>( in.raw_data() ) );
		
		__m128& pos00_m128	= *reinterpret_cast<__m128 *>( out[0].raw_data() );
		__m128& pos01_m128	= *reinterpret_cast<__m128 *>( out[1].raw_data() );
		__m128& pos10_m128	= *reinterpret_cast<__m128 *>( out[2].raw_data() );
		__m128& pos11_m128	= *reinterpret_cast<__m128 *>( out[3].raw_data() );

		__m128  step0_m128	= _mm_load_ps1(&step0);
		__m128  step1_m128	= _mm_load_ps1(&step1);

		pos00_m128 = _mm_add_ps(
			in_m128,
			_mm_add_ps( _mm_mul_ps(d0_m128, step0_m128), _mm_mul_ps(d1_m128, step1_m128) )
			);
		pos01_m128 = _mm_add_ps(pos00_m128, d0_m128);
		pos10_m128 = _mm_add_ps(pos00_m128, d1_m128);
		pos11_m128 = _mm_add_ps(pos01_m128, d1_m128);
#else
		EFLIB_ASSERT_UNIMPLEMENTED();
#endif

		return *out;
	}

	template <int N>
	vs_output& step_2d_unproj_attr_n_quad(
		vs_output* out, const vs_output& in,
		float step0, const vs_output& derivation0,
		float step1, const vs_output& derivation1
        )
	{
#if defined(VSO_INTERP_SSE_ENABLED)
		__m128 const* d0_m128		= reinterpret_cast<__m128 const*>( derivation0.raw_data() );
		__m128 const* d1_m128		= reinterpret_cast<__m128 const*>( derivation1.raw_data() );

		__m128 const* in_m128		= reinterpret_cast<__m128 const*>( in.raw_data() );
		
		__m128*		  out00_m128	= reinterpret_cast<__m128 *>( out[0].raw_data() );
		__m128*		  out01_m128	= reinterpret_cast<__m128 *>( out[1].raw_data() );
		__m128*		  out10_m128	= reinterpret_cast<__m128 *>( out[2].raw_data() );
		__m128*		  out11_m128	= reinterpret_cast<__m128 *>( out[3].raw_data() );

		__m128		  step0_m128	= _mm_load_ps1(&step0);
		__m128		  step1_m128	= _mm_load_ps1(&step1);

		EFLIB_ALIGN(16) float w[] = 
		{
			_xmm_extract_ps(out00_m128[0], 3),
			_xmm_extract_ps(out01_m128[0], 3),
			_xmm_extract_ps(out10_m128[0], 3),
			_xmm_extract_ps(out11_m128[0], 3)
		};

		__m128 inv_w4 = _mm_set_ps(
			1.0f/w[0],
			1.0f/w[1],
			1.0f/w[2],
			1.0f/w[3]
			);

		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			__m128 interp_attr00;
			__m128 interp_attr01;
			__m128 interp_attr10;
			__m128 interp_attr11;

			if (vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_nointerpolation)
			{
				interp_attr00 = in_m128[i_attr+1];
				interp_attr01 = in_m128[i_attr+1];
				interp_attr10 = in_m128[i_attr+1];
				interp_attr11 = in_m128[i_attr+1];
			}
			else
			{
				interp_attr00 = _mm_add_ps(
					in_m128[i_attr + 1],
					_mm_add_ps(
						_mm_mul_ps(d0_m128[i_attr + 1], step0_m128),
						_mm_mul_ps(d1_m128[i_attr + 1], step1_m128)
						)
					);
				interp_attr01 = _mm_add_ps(interp_attr00, d0_m128[i_attr+1]);
				interp_attr10 = _mm_add_ps(interp_attr00, d1_m128[i_attr+1]);
				interp_attr11 = _mm_add_ps(interp_attr01, d1_m128[i_attr+1]);
			}

			// Perspective
			if (vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_noperspective)
			{
				out00_m128[i_attr+1] = interp_attr00;
				out01_m128[i_attr+1] = interp_attr01;
				out10_m128[i_attr+1] = interp_attr10;
				out11_m128[i_attr+1] = interp_attr11;
			}
			else
			{
				out00_m128[i_attr+1] = _mm_mul_ps(interp_attr00, _mm_shuffle_ps(inv_w4, inv_w4, _MM_SHUFFLE(3, 3, 3, 3)));
				out01_m128[i_attr+1] = _mm_mul_ps(interp_attr01, _mm_shuffle_ps(inv_w4, inv_w4, _MM_SHUFFLE(2, 2, 2, 2)));
				out10_m128[i_attr+1] = _mm_mul_ps(interp_attr10, _mm_shuffle_ps(inv_w4, inv_w4, _MM_SHUFFLE(1, 1, 1, 1)));
				out11_m128[i_attr+1] = _mm_mul_ps(interp_attr11, _mm_shuffle_ps(inv_w4, inv_w4, _MM_SHUFFLE(0, 0, 0, 0)));
			}
		}
#else
		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			if (vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_nointerpolation)
			{
				out.attribute(i_attr) = in.attribute(i_attr);
			}
			else
			{
				out.attribute(i_attr) =
					in.attribute(i_attr)
					+ (derivation0.attribute(i_attr) * step0)
					+ (derivation1.attribute(i_attr) * step1);
			}
		}
#endif

		return *out;
	}

	template <int N>
	vs_output& add_n(vs_output& out, const vs_output& vso0, const vs_output& vso1)
	{
		out.position() = vso0.position() + vso1.position();
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			out.attribute(i_attr) = vso0.attribute(i_attr) + vso1.attribute(i_attr);
		}
		return out;
	}
	
	template <int N>
	vs_output& sub_n(vs_output& out, const vs_output& vso0, const vs_output& vso1)
	{
#if defined(VSO_INTERP_SSE_ENABLED)
		__m128 const* v1_m128 = reinterpret_cast<__m128 const*>( vso1.raw_data() );
		__m128 const* v0_m128 = reinterpret_cast<__m128 const*>( vso0.raw_data() );

		__m128* out_m128 = reinterpret_cast<__m128*>( out.raw_data() );

		for(size_t register_index = 0; register_index < N+1; ++register_index)
		{
			out_m128[register_index] =
				_mm_sub_ps(v0_m128[register_index], v1_m128[register_index]);
		}
#else
		out.position() = vso0.position() - vso1.position();
		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			out.attribute(i_attr)
				= vso0.attribute(i_attr) - vso1.attribute(i_attr);
		}
#endif

		return out;
	}
	
	template <int N>
	vs_output& mul_n(vs_output& out, const vs_output& vso0, float f)
	{
		out.position() = vso0.position() * f;
		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			out.attribute(i_attr) = vso0.attribute(i_attr) * f;
		}
		return out;
	}

	template <int N>
	vs_output& div_n(vs_output& out, const vs_output& vso0, float f)
	{
		return mul_n<N>(out, vso0, 1 / f);
	}

	template <int N>
	void compute_derivative_n(vs_output& ddx, vs_output& ddy, vs_output const& e01, vs_output const& e02, float inv_area)
	{
		// ddx = (e02 * e01.position.y - e02.position.y * e01) * inv_area;
		// ddy = (e01 * e02.position.x - e01.position.x * e02) * inv_area;

#if !defined(EFLIB_NO_SIMD)
		__m128*        mddx  = reinterpret_cast<__m128*>( ddx.raw_data() );
		__m128*        mddy  = reinterpret_cast<__m128*>( ddy.raw_data() );
		__m128 const * me01  = reinterpret_cast<__m128 const*>( e01.raw_data() );
		__m128 const * me02  = reinterpret_cast<__m128 const*>( e02.raw_data() );

		__m128 me01x = _mm_shuffle_ps(me01[0], me01[0], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 me01y = _mm_shuffle_ps(me01[0], me01[0], _MM_SHUFFLE(1, 1, 1, 1));
		
		__m128 me02x = _mm_shuffle_ps(me02[0], me02[0], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 me02y = _mm_shuffle_ps(me02[0], me02[0], _MM_SHUFFLE(1, 1, 1, 1));

		__m128 minv_area = _mm_set_ps1(inv_area);

		for(int i = 0; i < N + 1; ++i)
		{
			__m128 x_diff = _mm_sub_ps(
				_mm_mul_ps(me02[i], me01y),
				_mm_mul_ps(me01[i], me02y)
				);

			__m128 y_diff = _mm_sub_ps(
				_mm_mul_ps(me01[i], me02x),
				_mm_mul_ps(me02[i], me01x)
				);

			mddx[i] = _mm_mul_ps(x_diff, minv_area);
			mddy[i] = _mm_mul_ps(y_diff, minv_area);
		}
#else
		for(int i = 0; i < N + 1; ++i)
		{
			ddx.raw_data()[i] = inv_area * ( e02.raw_data()[i] * e01.position().y() - e01.raw_data()[i] * e02.position().y() );
			ddy.raw_data()[i] = inv_area * ( e01.raw_data()[i] * e02.position().x() - e02.raw_data()[i] * e01.position().x() );
		}
#endif
	}
}

template <int N>
vs_input_op gen_vs_input_op_n()
{
	using namespace vs_input_op_funcs;

	vs_input_op ret;

	ret.construct = construct_n<N>;
	ret.copy = copy_n<N>;

	return ret;
}

template <int N>
vs_output_op gen_vs_output_op_n()
{
	using namespace vs_output_op_funcs;

	vs_output_op ret;

	ret.construct	= construct_n<N>;
	ret.copy		= copy_n<N>;

	ret.project		= project_n<N>;
	ret.unproject	= unproject_n<N>;

	ret.add = add_n<N>;
	ret.sub = sub_n<N>;
	ret.mul = mul_n<N>;
	ret.div = div_n<N>;

	ret.lerp = lerp_n<N>;
    ret.step_2d_unproj_pos = step_2d_unproj_pos;
    ret.step_2d_unproj_attr = step_2d_unproj_attr_n<N>;
	ret.step_2d_unproj_pos_quad = step_2d_unproj_pos_quad;
	ret.step_2d_unproj_attr_quad = step_2d_unproj_attr_n_quad<N>;
	ret.compute_derivative = compute_derivative_n<N>;

	return ret;
}

vs_input_op& get_vs_input_op(uint32_t n)
{
	return vs_input_ops[n];
}

vs_output_op& get_vs_output_op(uint32_t n)
{
	return vs_output_ops[n];
}

void viewport_transform(vec4& position, const viewport& vp)
{
	float invw = (eflib::equal<float>(position[3], 0.0f)) ? 1.0f : 1.0f / position[3];
	vec4 pos = position * invw;

	// Transform to viewport space.
	float ox = (vp.x + vp.w) * 0.5f;
	float oy = (vp.y + vp.h) * 0.5f;

	position[0] = (float(vp.w) * 0.5f) *  pos.x() + ox;
	position[1] = (float(vp.h) * 0.5f) * -pos.y() + oy;
	position[2] = (vp.maxz - vp.minz) * pos.z() + vp.minz;
	position[3] = invw;
}

float compute_area(const vs_output& v0, const vs_output& v1, const vs_output& v2)
{
	return cross_prod2( ( v1.position() - v0.position() ).xy(), (v2.position() - v0.position() ).xy() );
}

/*****************************************
 *  Vertex Shader
 ****************************************/
void cpp_vertex_shader::execute(const vs_input& in, vs_output& out){
	shader_prog(in, out);
}

void cpp_blend_shader::execute(size_t sample, pixel_accessor& out, const ps_output& in)
{
	shader_prog(sample, out, in);
}

result cpp_shader_impl::find_register( semantic_value const& sv, size_t& index )
{
	register_map::const_iterator it = regmap_.find( sv );
	if( it != regmap_.end() ){
		index = it->second;
		return result::ok;
	}
	return result::failed;
}

boost::unordered_map<semantic_value, size_t> const& cpp_shader_impl::get_register_map()
{
	return regmap_;
}

void cpp_shader_impl::bind_semantic( char const* name, size_t semantic_index, size_t register_index )
{
	bind_semantic( semantic_value(name, static_cast<uint32_t>(semantic_index)), register_index );
}

void cpp_shader_impl::bind_semantic( semantic_value const& s, size_t register_index )
{
	regmap_.insert( make_pair(s, register_index) );
}

END_NS_SALVIAR();
