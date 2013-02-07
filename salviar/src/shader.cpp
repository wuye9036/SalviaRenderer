#include <salviar/include/shaderregs.h>
#include <salviar/include/shaderregs_op.h>
#include <salviar/include/shader.h>
#include <salviar/include/renderer.h>

BEGIN_NS_SALVIAR();

using namespace boost;
using namespace eflib;

using std::make_pair;

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

	ret.self_add = self_add_n<N>;
	ret.self_sub = self_sub_n<N>;
	ret.self_mul = self_mul_n<N>;
	ret.self_div = self_div_n<N>;
	
	ret.add = add_n<N>;
	ret.sub = sub_n<N>;
	ret.mul = mul_n<N>;
	ret.div = div_n<N>;

	ret.lerp = lerp_n<N>;
	ret.step_unproj = step_unproj_n<N>;
	
	ret.step1		= step1_n<N>;
	ret.step_1d		= step_1d_n<N>;
	ret.step_2d		= step_2d_n<N>;

	ret.self_step1	= self_step1_n<N>;
	ret.self_step_1d= self_step_1d_n<N>;

	return ret;
}

vs_input_op vs_input_ops[vso_attribute_count] = {
	gen_vs_input_op_n<0>(),
	gen_vs_input_op_n<1>(),
	gen_vs_input_op_n<2>(),
	gen_vs_input_op_n<3>(),
	gen_vs_input_op_n<4>(),
	gen_vs_input_op_n<5>(),
	gen_vs_input_op_n<6>(),
	gen_vs_input_op_n<7>(),
	gen_vs_input_op_n<8>(),
	gen_vs_input_op_n<9>(),
	gen_vs_input_op_n<10>(),
	gen_vs_input_op_n<11>(),
	gen_vs_input_op_n<12>(),
	gen_vs_input_op_n<13>(),
	gen_vs_input_op_n<14>(),
	gen_vs_input_op_n<15>(),
	gen_vs_input_op_n<16>(),
	gen_vs_input_op_n<17>(),
	gen_vs_input_op_n<18>(),
	gen_vs_input_op_n<19>(),
	gen_vs_input_op_n<20>(),
	gen_vs_input_op_n<21>(),
	gen_vs_input_op_n<22>(),
	gen_vs_input_op_n<23>(),
	gen_vs_input_op_n<24>(),
	gen_vs_input_op_n<25>(),
	gen_vs_input_op_n<26>(),
	gen_vs_input_op_n<27>(),
	gen_vs_input_op_n<28>(),
	gen_vs_input_op_n<29>(),
	gen_vs_input_op_n<30>(),
	gen_vs_input_op_n<31>()
};

vs_output_op vs_output_ops[vso_attribute_count] = {
	gen_vs_output_op_n<0>(),
	gen_vs_output_op_n<1>(),
	gen_vs_output_op_n<2>(),
	gen_vs_output_op_n<3>(),
	gen_vs_output_op_n<4>(),
	gen_vs_output_op_n<5>(),
	gen_vs_output_op_n<6>(),
	gen_vs_output_op_n<7>(),
	gen_vs_output_op_n<8>(),
	gen_vs_output_op_n<9>(),
	gen_vs_output_op_n<10>(),
	gen_vs_output_op_n<11>(),
	gen_vs_output_op_n<12>(),
	gen_vs_output_op_n<13>(),
	gen_vs_output_op_n<14>(),
	gen_vs_output_op_n<15>(),
	gen_vs_output_op_n<16>(),
	gen_vs_output_op_n<17>(),
	gen_vs_output_op_n<18>(),
	gen_vs_output_op_n<19>(),
	gen_vs_output_op_n<20>(),
	gen_vs_output_op_n<21>(),
	gen_vs_output_op_n<22>(),
	gen_vs_output_op_n<23>(),
	gen_vs_output_op_n<24>(),
	gen_vs_output_op_n<25>(),
	gen_vs_output_op_n<26>(),
	gen_vs_output_op_n<27>(),
	gen_vs_output_op_n<28>(),
	gen_vs_output_op_n<29>(),
	gen_vs_output_op_n<30>(),
	gen_vs_output_op_n<31>()
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
			const eflib::vec4& position, bool front_face,
			eflib::vec4 const* attribs)
	{
		out.position() = position;
		out.front_face(front_face);
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
		out.front_face( in.front_face() );
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
			out.front_face( in.front_face() );
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
		__m128*			dst = NULL;
		__m128 const*	src = NULL;

		const float inv_w = 1.0f / in.position().w();
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
		out.front_face( in.front_face() );

		return out;
	}

	template <int N>
	vs_output& lerp_n(vs_output& out, const vs_output& start, const vs_output& end, float step)
	{
		out.position() = start.position() + ( end.position() - start.position() ) * step;
		out.front_face( start.front_face() );
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			out.attribute(i_attr) = start.attribute(i_attr);
			if (!(vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
				out.attribute(i_attr) += (end.attribute(i_attr) - start.attribute(i_attr)) * step;
			}
		}
		return out;
	}

	template <int N>
	vs_output& step_unproj_n(vs_output& out, vs_output const& start, vs_output const& derivation)
	{
#if ( defined(EFLIB_CPU_X86) || defined(EFLIB_CPU_X64) ) && !defined(EFLIB_NO_SIMD)
		__m128 const* deri_m128  = reinterpret_cast<__m128 const*>( derivation.raw_data() );
		__m128 const* start_m128 = reinterpret_cast<__m128 const*>( start.raw_data() );
		__m128*		  out_m128   = reinterpret_cast<__m128 *>( out.raw_data() );

		// Position
		out_m128[0] = _mm_add_ps(start_m128[0], deri_m128[0]);
		float inv_w = out_m128[0].m128_f32[3];
		__m128 inv_w4 = _mm_load_ps1(&inv_w);

		for(size_t register_index = 1; register_index <= N; ++register_index)
		{
			__m128 interp_attr;

			// Interpolation
			if (vs_output_ops[N].attribute_modifiers[register_index-1] & vs_output::am_nointerpolation)
			{
				interp_attr = start_m128[register_index];
			}
			else
			{
				interp_attr = _mm_add_ps(start_m128[register_index], deri_m128[register_index]);
			}

			// Perspective
			if (vs_output_ops[N].attribute_modifiers[register_index-1] & vs_output::am_noperspective)
			{
				out_m128[register_index] = interp_attr;
			}
			else
			{
				out_m128[register_index] = _mm_mul_ps(interp_attr, inv_w4);
			}

			// Face
			out.front_face( start.front_face() );
		}

		return out;
#else
		vs_output tmp;
		integral1_n<N>(tmp, start, derivation);
		unproject_n(out, tmp);
		return out;
#endif
	}

	template <int N>
	vs_output& step1_n(vs_output& out, const vs_output& in, const vs_output& derivation)
	{
		out.position() = in.position() + derivation.position();
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			if (!(vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
				out.attribute(i_attr) = in.attribute(i_attr) + derivation.attribute(i_attr);
			}
		}
		return out;
	}
	template <int N>
	vs_output& step_1d_n(vs_output& out, const vs_output& in, float step, const vs_output& derivation)
	{
		out.position() = in.position() + (derivation.position() * step);
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			if (!(vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
				out.attribute(i_attr) = in.attribute(i_attr) + (derivation.attribute(i_attr) * step);
			}
		}
		return out;
	}
	template <int N>
	vs_output& step_2d_n(
		vs_output& out, const vs_output& in,
		float step0, const vs_output& derivation0,
		float step1, const vs_output& derivation1)
	{
#if ( defined(EFLIB_CPU_X86) || defined(EFLIB_CPU_X64) ) && !defined(EFLIB_NO_SIMD)
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

		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			if (vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_nointerpolation)
			{
				out_m128[i_attr+1] = in_m128[i_attr+1];
			}
			else
			{
				out_m128[i_attr+1] = _mm_add_ps(
					in_m128[i_attr+1],
					_mm_add_ps(
						_mm_mul_ps(d0_m128[i_attr+1], step0_m128),
						_mm_mul_ps(d1_m128[i_attr+1], step1_m128)
						)
					);
			}
		}
#else
		out.position() =
			in.position()
			+ (derivation0.position() * step0)
			+ (derivation1.position() * step1);

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

	template <int N>
	vs_output& self_step1_n(vs_output& inout, const vs_output& derivation)
	{
#if defined(EFLIB_CPU_X86) || defined(EFLIB_CPU_X64) && !defined(EFLIB_NO_SIMD)
		__m128 const*	src = NULL;
		__m128*			dst = NULL;

		src = reinterpret_cast<__m128 const*>( &derivation.position() );
		dst = reinterpret_cast<__m128*>( &inout.position() );
		*dst = _mm_add_ps(*src, *dst);

		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			uint32_t modifier
				= vs_output_ops[N].attribute_modifiers[i_attr];
			if (!(modifier & vs_output::am_nointerpolation) )
			{
				src = reinterpret_cast<__m128 const*>( &derivation.attribute(i_attr) );
				dst = reinterpret_cast<__m128*>( &inout.attribute(i_attr) );

				*dst = _mm_add_ps(*src, *dst);
			}
		}
#else
		inout.position() += derivation.position();
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			if (!(vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
				inout.attribute(i_attr) += derivation.attribute(i_attr);
			}
		}
#endif
		return inout;
	}
	template <int N>
	vs_output& self_step_1d_n(vs_output& inout, float step, const vs_output& derivation)
	{
		inout.position() += (derivation.position() * step);
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			if (!(vs_output_ops[N].attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
				inout.attribute(i_attr) += (derivation.attribute(i_attr) * step);
			}
		}
		return inout;
	}

	template <int N>
	vs_output& self_add_n(vs_output& lhs, const vs_output& rhs)
	{
		lhs.position() += rhs.position();
		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			lhs.attribute(i_attr) += rhs.attribute(i_attr);
		}
		return lhs;
	}

	template <int N>
	vs_output& self_sub_n(vs_output& lhs, const vs_output& rhs)
	{
		lhs.position() -= rhs.position();
		for(size_t i_attr = 0; i_attr < N; ++i_attr)
		{
			lhs.attribute(i_attr) -= rhs.attribute(i_attr);
		}
		return lhs;
	}
	
	template <int N>
	vs_output& self_mul_n(vs_output& lhs, float f)
	{
		lhs.position() *= f;
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			lhs.attribute(i_attr) *= f;
		}
		return lhs;
	}
	template <int N>
	vs_output& self_div_n(vs_output& lhs, float f)
	{
		assert( !eflib::equal<float>(f, 0.0f) );
		return self_mul_n<N>(lhs, 1 / f);
	}

	template <int N>
	vs_output& add_n(vs_output& out, const vs_output& vso0, const vs_output& vso1)
	{
		out.position() = vso0.position() + vso1.position();
		out.front_face( vso0.front_face() );
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			out.attribute(i_attr) = vso0.attribute(i_attr) + vso1.attribute(i_attr);
		}
		return out;
	}
	template <int N>
	vs_output& sub_n(vs_output& out, const vs_output& vso0, const vs_output& vso1)
	{
		__m128 const* v1_m128 = reinterpret_cast<__m128 const*>( vso1.raw_data() );
		__m128 const* v0_m128 = reinterpret_cast<__m128 const*>( vso0.raw_data() );

		__m128* out_m128 = reinterpret_cast<__m128*>( out.raw_data() );

		for(size_t register_index = 0; register_index < N+1; ++register_index)
		{
			out_m128[register_index] =
				_mm_sub_ps(v0_m128[register_index], v1_m128[register_index]);
		}
		out.front_face( vso0.front_face() );
		return out;
	}
	template <int N>
	vs_output& mul_n(vs_output& out, const vs_output& vso0, float f)
	{
		out.position() = vso0.position() * f;
		out.front_face( vso0.front_face() );
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			out.attribute(i_attr) = vso0.attribute(i_attr) * f;
		}
		return out;
	}

	template <int N>
	vs_output& div_n(vs_output& out, const vs_output& vso0, float f)
	{
		return mul_n<N>(out, vso0, 1 / f);
	}
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
	position[2] = (vp.maxz - vp.minz) * 0.5f * pos.z() + vp.minz;
	position[3] = invw;
}

float compute_area(const vs_output& v0, const vs_output& v1, const vs_output& v2)
{
	return cross_prod2( ( v1.position() - v0.position() ).xy(), (v2.position() - v0.position() ).xy() );
}

/*****************************************
 *  Vertex Shader
 ****************************************/
void vertex_shader::execute(const vs_input& in, vs_output& out){
	shader_prog(in, out);
}

void blend_shader::execute(size_t sample, backbuffer_pixel_out& out, const ps_output& in){
	shader_prog(sample, out, in);
}

result shader_impl::find_register( semantic_value const& sv, size_t& index ){
	register_map::const_iterator it = regmap_.find( sv );
	if( it != regmap_.end() ){
		index = it->second;
		return result::ok;
	}
	return result::failed;
}

boost::unordered_map<semantic_value, size_t> const& shader_impl::get_register_map(){
	return regmap_;
}

void shader_impl::bind_semantic( char const* name, size_t semantic_index, size_t register_index ){
	bind_semantic( semantic_value(name, static_cast<uint32_t>(semantic_index)), register_index );
}

void shader_impl::bind_semantic( semantic_value const& s, size_t register_index ){
	regmap_.insert( make_pair(s, register_index) );
}

END_NS_SALVIAR();
