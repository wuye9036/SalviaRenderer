#include "../include/shaderregs_op.h"
#include "../include/shader.h"
#include "../include/renderer.h"
BEGIN_NS_SOFTART()


using namespace boost;
using namespace eflib;

template <int N>
void construct_n(vs_output& out,
		const eflib::vec4& position, bool front_face,
		const vs_output::attrib_array_type& attribs,
		const vs_output::attrib_modifier_array_type& modifiers)
{
	out.position = position;
	out.front_face = front_face;
	out.num_used_attribute = N;
	for(size_t i_attr = 0; i_attr < N; ++i_attr){
		out.attributes[i_attr] = attribs[i_attr];
		out.attribute_modifiers[i_attr] = modifiers[i_attr];
	}
}
template <>
void construct_n<0>(vs_output& out,
		const eflib::vec4& /*position*/, bool /*front_face*/,
		const vs_output::attrib_array_type& /*attribs*/,
		const vs_output::attrib_modifier_array_type& /*modifiers*/)
{
	out.num_used_attribute = 0;
}
template <int N>
void copy_n(vs_output& out, const vs_output& in)
{
	out.position = in.position;
	out.front_face = in.front_face;
	out.num_used_attribute = N;
	for(size_t i_attr = 0; i_attr < N; ++i_attr){
		out.attributes[i_attr] = in.attributes[i_attr];
		out.attribute_modifiers[i_attr] = in.attribute_modifiers[i_attr];
	}
}
template <>
void copy_n<0>(vs_output& out, const vs_output& /*in*/)
{
	out.num_used_attribute = 0;
}
template <int N>
vs_output project1_n(const vs_output& in)
{
	vs_output::attrib_array_type ret_attribs;
	vs_output::attrib_modifier_array_type ret_attrib_modifiers;
	for(size_t i_attr = 0; i_attr < N; ++i_attr){
		ret_attribs[i_attr] = in.attributes[i_attr];
		ret_attrib_modifiers[i_attr] = in.attribute_modifiers[i_attr];
		if (!(in.attribute_modifiers[i_attr] & vs_output::am_noperspective)){
			ret_attribs[i_attr] *= in.position.w;
		}
	}
	return vs_output(in.position, in.front_face, ret_attribs, ret_attrib_modifiers, N);
}
template <>
vs_output project1_n<0>(const vs_output& /*in*/)
{
	return vs_output();
}
template <int N>
vs_output& project2_n(vs_output& out, const vs_output& in)
{
	if (&out != &in){
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			out.attributes[i_attr] = in.attributes[i_attr];
			out.attribute_modifiers[i_attr] = in.attribute_modifiers[i_attr];
			if (!(in.attribute_modifiers[i_attr] & vs_output::am_noperspective)){
				out.attributes[i_attr] *= in.position.w;
			}
		}
		out.num_used_attribute = N;
		out.position = in.position;
		out.front_face = in.front_face;
	}
	else{
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			if (!(in.attribute_modifiers[i_attr] & vs_output::am_noperspective)){
				out.attributes[i_attr] *= in.position.w;
			}
		}
	}
	return out;
}
template <>
vs_output& project2_n<0>(vs_output& out, const vs_output& /*in*/)
{
	out.num_used_attribute = 0;
	return out;
}
template <int N>
vs_output unproject1_n(const vs_output& in)
{
	vs_output::attrib_array_type ret_attribs;
	vs_output::attrib_modifier_array_type ret_attrib_modifiers;
	const float inv_w = 1.0f / in.position.w;
	for(size_t i_attr = 0; i_attr < N; ++i_attr){
		ret_attribs[i_attr] = in.attributes[i_attr];
		ret_attrib_modifiers[i_attr] = in.attribute_modifiers[i_attr];
		if (!(in.attribute_modifiers[i_attr] & vs_output::am_noperspective)){
			ret_attribs[i_attr] *= inv_w;
		}
	}
	return vs_output(in.position, in.front_face, ret_attribs, ret_attrib_modifiers, N);
}
template <>
vs_output unproject1_n<0>(const vs_output& /*in*/)
{
	return vs_output();
}
template <int N>
vs_output& unproject2_n(vs_output& out, const vs_output& in)
{
	const float inv_w = 1.0f / in.position.w;
	if (&out != &in){
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			out.attributes[i_attr] = in.attributes[i_attr];
			out.attribute_modifiers[i_attr] = in.attribute_modifiers[i_attr];
			if (!(in.attribute_modifiers[i_attr] & vs_output::am_noperspective)){
				out.attributes[i_attr] *= inv_w;
			}
		}
		out.num_used_attribute = N;
		out.position = in.position;
		out.front_face = in.front_face;
	}
	else{
		for(size_t i_attr = 0; i_attr < N; ++i_attr){
			if (!(in.attribute_modifiers[i_attr] & vs_output::am_noperspective)){
				out.attributes[i_attr] *= inv_w;
			}
		}
	}
	return out;
}
template <>
vs_output& unproject2_n<0>(vs_output& out, const vs_output& /*in*/)
{
	out.num_used_attribute = 0;
	return out;
}
template <int N>
vs_output& integral1_n(vs_output& inout, const vs_output& derivation)
{
	EFLIB_ASSERT(inout.num_used_attribute == derivation.num_used_attribute, "");

	inout.position += derivation.position;
	for(size_t i_attr = 0; i_attr < N; ++i_attr){
		EFLIB_ASSERT(inout.attribute_modifiers[i_attr] == derivation.attribute_modifiers[i_attr], "");
		if (!(inout.attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
			inout.attributes[i_attr] += derivation.attributes[i_attr];
		}
	}
	return inout;
}
template <>
vs_output& integral1_n<0>(vs_output& inout, const vs_output& derivation)
{
	inout.position += derivation.position;
	return inout;
}
template <int N>
vs_output& integral2_n(vs_output& inout, float step, const vs_output& derivation)
{
	EFLIB_ASSERT(inout.num_used_attribute == derivation.num_used_attribute, "");

	inout.position += (derivation.position * step);
	for(size_t i_attr = 0; i_attr < N; ++i_attr){
		EFLIB_ASSERT(inout.attribute_modifiers[i_attr] == derivation.attribute_modifiers[i_attr], "");
		if (!(inout.attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
			inout.attributes[i_attr] += (derivation.attributes[i_attr] * step);
		}
	}
	return inout;
}
template <>
vs_output& integral2_n<0>(vs_output& inout, float step, const vs_output& derivation)
{
	inout.position += (derivation.position * step);
	return inout;
}
template <int N>
vs_output operator_sub_n(const vs_output& vso0, const vs_output& vso1)
{
	vs_output::attrib_array_type ret_attribs;
	for(size_t i_attr = 0; i_attr < N; ++i_attr){
		EFLIB_ASSERT(vso0.attribute_modifiers[i_attr] == vso1.attribute_modifiers[i_attr], "");
		ret_attribs[i_attr] = vso0.attributes[i_attr] - vso1.attributes[i_attr];
	}

	return vs_output(vso0.position - vso1.position, vso0.front_face, ret_attribs, vso0.attribute_modifiers, N);
}
template <>
vs_output operator_sub_n<0>(const vs_output& /*vso0*/, const vs_output& /*vso1*/)
{
	return vs_output();
}

struct vs_output_op
{
	typedef void (*vs_output_construct)(vs_output& out,
		const eflib::vec4& position, bool front_face,
		const vs_output::attrib_array_type& attribs,
		const vs_output::attrib_modifier_array_type& modifiers);
	typedef void (*vs_output_copy)(vs_output& out, const vs_output& in);
	typedef vs_output (*vs_output_project1)(const vs_output& in);
	typedef vs_output& (*vs_output_project2)(vs_output& out, const vs_output& in);
	typedef vs_output (*vs_output_unproject1)(const vs_output& in);
	typedef vs_output& (*vs_output_unproject2)(vs_output& out, const vs_output& in);
	typedef vs_output (*vs_output_operator_sub)(const vs_output& vso0, const vs_output& vso1);

	typedef vs_output& (*vs_output_integral1)(vs_output& inout, const vs_output& derivation);
	typedef vs_output& (*vs_output_integral2)(vs_output& inout, float step, const vs_output& derivation);

	vs_output_construct construct;
	vs_output_copy copy;

	vs_output_project1 project1;
	vs_output_project2 project2;

	vs_output_unproject1 unproject1;
	vs_output_unproject2 unproject2;

	vs_output_integral1 integral1;
	vs_output_integral2 integral2;

	vs_output_operator_sub operator_sub;
};

vs_output_op vs_output_ops[vso_attrib_regcnt] = {
	{ construct_n<0>, copy_n<0>, project1_n<0>, project2_n<0>, unproject1_n<0>, unproject2_n<0>, integral1_n<0>, integral2_n<0>, operator_sub_n<0> },
	{ construct_n<1>, copy_n<1>, project1_n<1>, project2_n<1>, unproject1_n<1>, unproject2_n<1>, integral1_n<1>, integral2_n<1>, operator_sub_n<1> },
	{ construct_n<2>, copy_n<2>, project1_n<2>, project2_n<2>, unproject1_n<2>, unproject2_n<2>, integral1_n<2>, integral2_n<2>, operator_sub_n<2> },
	{ construct_n<3>, copy_n<3>, project1_n<3>, project2_n<3>, unproject1_n<3>, unproject2_n<3>, integral1_n<3>, integral2_n<3>, operator_sub_n<3> },
	{ construct_n<4>, copy_n<4>, project1_n<4>, project2_n<4>, unproject1_n<4>, unproject2_n<4>, integral1_n<4>, integral2_n<4>, operator_sub_n<4> },
	{ construct_n<5>, copy_n<5>, project1_n<5>, project2_n<5>, unproject1_n<5>, unproject2_n<5>, integral1_n<5>, integral2_n<5>, operator_sub_n<5> },
	{ construct_n<6>, copy_n<6>, project1_n<6>, project2_n<6>, unproject1_n<6>, unproject2_n<6>, integral1_n<6>, integral2_n<6>, operator_sub_n<6> },
	{ construct_n<7>, copy_n<7>, project1_n<7>, project2_n<7>, unproject1_n<7>, unproject2_n<7>, integral1_n<7>, integral2_n<7>, operator_sub_n<7> },
	{ construct_n<8>, copy_n<8>, project1_n<8>, project2_n<8>, unproject1_n<8>, unproject2_n<8>, integral1_n<8>, integral2_n<8>, operator_sub_n<8> },
	{ construct_n<9>, copy_n<9>, project1_n<9>, project2_n<9>, unproject1_n<9>, unproject2_n<9>, integral1_n<9>, integral2_n<9>, operator_sub_n<9> },
	{ construct_n<10>, copy_n<10>, project1_n<10>, project2_n<10>, unproject1_n<10>, unproject2_n<10>, integral1_n<10>, integral2_n<10>, operator_sub_n<10> },
	{ construct_n<11>, copy_n<11>, project1_n<11>, project2_n<11>, unproject1_n<11>, unproject2_n<11>, integral1_n<11>, integral2_n<11>, operator_sub_n<11> },
	{ construct_n<12>, copy_n<12>, project1_n<12>, project2_n<12>, unproject1_n<12>, unproject2_n<12>, integral1_n<12>, integral2_n<12>, operator_sub_n<12> },
	{ construct_n<13>, copy_n<13>, project1_n<13>, project2_n<13>, unproject1_n<13>, unproject2_n<13>, integral1_n<13>, integral2_n<13>, operator_sub_n<13> },
	{ construct_n<14>, copy_n<14>, project1_n<14>, project2_n<14>, unproject1_n<14>, unproject2_n<14>, integral1_n<14>, integral2_n<14>, operator_sub_n<14> },
	{ construct_n<15>, copy_n<15>, project1_n<15>, project2_n<15>, unproject1_n<15>, unproject2_n<15>, integral1_n<15>, integral2_n<15>, operator_sub_n<15> },
	{ construct_n<16>, copy_n<16>, project1_n<16>, project2_n<16>, unproject1_n<16>, unproject2_n<16>, integral1_n<16>, integral2_n<16>, operator_sub_n<16> },
	{ construct_n<17>, copy_n<17>, project1_n<17>, project2_n<17>, unproject1_n<17>, unproject2_n<17>, integral1_n<17>, integral2_n<17>, operator_sub_n<17> },
	{ construct_n<18>, copy_n<18>, project1_n<18>, project2_n<18>, unproject1_n<18>, unproject2_n<18>, integral1_n<18>, integral2_n<18>, operator_sub_n<18> },
	{ construct_n<19>, copy_n<19>, project1_n<19>, project2_n<19>, unproject1_n<19>, unproject2_n<19>, integral1_n<19>, integral2_n<19>, operator_sub_n<19> },
	{ construct_n<20>, copy_n<20>, project1_n<20>, project2_n<20>, unproject1_n<20>, unproject2_n<20>, integral1_n<20>, integral2_n<20>, operator_sub_n<20> },
	{ construct_n<21>, copy_n<21>, project1_n<21>, project2_n<21>, unproject1_n<21>, unproject2_n<21>, integral1_n<21>, integral2_n<21>, operator_sub_n<21> },
	{ construct_n<22>, copy_n<22>, project1_n<22>, project2_n<22>, unproject1_n<22>, unproject2_n<22>, integral1_n<22>, integral2_n<22>, operator_sub_n<22> },
	{ construct_n<23>, copy_n<23>, project1_n<23>, project2_n<23>, unproject1_n<23>, unproject2_n<23>, integral1_n<23>, integral2_n<23>, operator_sub_n<23> },
	{ construct_n<24>, copy_n<24>, project1_n<24>, project2_n<24>, unproject1_n<24>, unproject2_n<24>, integral1_n<24>, integral2_n<24>, operator_sub_n<24> },
	{ construct_n<25>, copy_n<25>, project1_n<25>, project2_n<25>, unproject1_n<25>, unproject2_n<25>, integral1_n<25>, integral2_n<25>, operator_sub_n<25> },
	{ construct_n<26>, copy_n<26>, project1_n<26>, project2_n<26>, unproject1_n<26>, unproject2_n<26>, integral1_n<26>, integral2_n<26>, operator_sub_n<26> },
	{ construct_n<27>, copy_n<27>, project1_n<27>, project2_n<27>, unproject1_n<27>, unproject2_n<27>, integral1_n<27>, integral2_n<27>, operator_sub_n<27> },
	{ construct_n<28>, copy_n<28>, project1_n<28>, project2_n<28>, unproject1_n<28>, unproject2_n<28>, integral1_n<28>, integral2_n<28>, operator_sub_n<28> },
	{ construct_n<29>, copy_n<29>, project1_n<29>, project2_n<29>, unproject1_n<29>, unproject2_n<29>, integral1_n<29>, integral2_n<29>, operator_sub_n<29> },
	{ construct_n<30>, copy_n<30>, project1_n<30>, project2_n<30>, unproject1_n<30>, unproject2_n<30>, integral1_n<30>, integral2_n<30>, operator_sub_n<30> },
	{ construct_n<31>, copy_n<31>, project1_n<31>, project2_n<31>, unproject1_n<31>, unproject2_n<31>, integral1_n<31>, integral2_n<31>, operator_sub_n<31> }
};

const eflib::vec4& vs_input::operator [](size_t i) const
{
	return attributes_[i];
}

eflib::vec4& vs_input::operator[](size_t i)
{
	if (num_used_attribute_ < i + 1)
	{
		num_used_attribute_ = static_cast<uint32_t>(i + 1);
	}

	return attributes_[i];
}

vs_output& vs_output::operator+=(const vs_output& rhs)
{
	assert(num_used_attribute == rhs.num_used_attribute);

	position += rhs.position;
	for(size_t i_attr = 0; i_attr < num_used_attribute; ++i_attr){
		assert(attribute_modifiers[i_attr] == rhs.attribute_modifiers[i_attr]);
		attributes[i_attr] += rhs.attributes[i_attr];
	}

	return *this;
}

vs_output& vs_output::operator-=(const vs_output& rhs)
{
	assert(num_used_attribute == rhs.num_used_attribute);

	position -= rhs.position;
	for(size_t i_attr = 0; i_attr < num_used_attribute; ++i_attr){
		assert(attribute_modifiers[i_attr] == rhs.attribute_modifiers[i_attr]);
		attributes[i_attr] -= rhs.attributes[i_attr];
	}

	return *this;
}

vs_output& vs_output::operator*=(float f)
{
	position *= f;
	for(size_t i_attr = 0; i_attr < num_used_attribute; ++i_attr){
		attributes[i_attr] *= f;
	}

	return *this;
}

vs_output& vs_output::operator/=(float f)
{
	EFLIB_ASSERT(!eflib::equal<float>(f, 0.0f), "");
	return operator*=(1.0f / f);
}

/**************************************
 *  Vertex Shader Output
 *************************************/
vs_output operator + (const vs_output& vso0, const vs_output& vso1)
{
	return vs_output(vso0) += vso1;
}

vs_output operator - (const vs_output& vso0, const vs_output& vso1)
{
	EFLIB_ASSERT(vso0.num_used_attribute == vso1.num_used_attribute, "");
	return vs_output_ops[vso0.num_used_attribute].operator_sub(vso0, vso1);
}

vs_output operator * (const vs_output& vso0, float f)
{
	return vs_output(vso0) *= f;
}

vs_output operator * (float f, const vs_output& vso0)
{
	return operator*(vso0, f);
}

vs_output operator / (const vs_output& vso0, float f)
{
	return vs_output(vso0) /= f;
}

vs_output project(const vs_output& in)
{
	return vs_output_ops[in.num_used_attribute].project1(in);
}

vs_output& project(vs_output& out, const vs_output& in)
{
	return vs_output_ops[in.num_used_attribute].project2(out, in);
}

vs_output unproject(const vs_output& in)
{
	return vs_output_ops[in.num_used_attribute].unproject1(in);
}

vs_output& unproject(vs_output& out, const vs_output& in)
{
	return vs_output_ops[in.num_used_attribute].unproject2(out, in);
}

vs_output lerp(const vs_output& start, const vs_output& end, float step)
{
	assert(start.num_used_attribute == end.num_used_attribute);

	vs_output out;
	out.position = start.position + (end.position - start.position) * step;
	out.front_face = start.front_face;
	for(size_t i_attr = 0; i_attr < start.num_used_attribute; ++i_attr){
		assert(start.attribute_modifiers[i_attr] == end.attribute_modifiers[i_attr]);

		out.attributes[i_attr] = start.attributes[i_attr];
		out.attribute_modifiers[i_attr] = start.attribute_modifiers[i_attr];
		if (!(start.attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
			out.attributes[i_attr] += (end.attributes[i_attr] - start.attributes[i_attr]) * step;
		}
	}
	out.num_used_attribute = start.num_used_attribute;

	return out;
}

vs_output& integral(vs_output& inout, const vs_output& derivation)
{
	return vs_output_ops[inout.num_used_attribute].integral1(inout, derivation);
}

vs_output& integral(vs_output& inout, float step, const vs_output& derivation)
{
	return vs_output_ops[inout.num_used_attribute].integral2(inout, step, derivation);
}

vs_output& integral(vs_output& out, const vs_output& in, const vs_output& derivation)
{
	assert(in.num_used_attribute == derivation.num_used_attribute);

	out.position = in.position + derivation.position;
	out.num_used_attribute = in.num_used_attribute;
	out.front_face = in.front_face;
	for(size_t i_attr = 0; i_attr < in.num_used_attribute; ++i_attr){
		assert(in.attribute_modifiers[i_attr] == derivation.attribute_modifiers[i_attr]);
		out.attribute_modifiers[i_attr] = in.attribute_modifiers[i_attr];
		if (!(in.attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
			out.attributes[i_attr] = in.attributes[i_attr] + derivation.attributes[i_attr];
		}
		else{
			out.attributes[i_attr] = in.attributes[i_attr];
		}
	}
	return out;
}

vs_output& integral(vs_output& out, const vs_output& in, float step, const vs_output& derivation)
{
	assert(in.num_used_attribute == derivation.num_used_attribute);

	out.position = in.position + (derivation.position * step);
	out.num_used_attribute = in.num_used_attribute;
	out.front_face = in.front_face;
	for(size_t i_attr = 0; i_attr < in.num_used_attribute; ++i_attr){
		assert(in.attribute_modifiers[i_attr] == derivation.attribute_modifiers[i_attr]);
		out.attribute_modifiers[i_attr] = in.attribute_modifiers[i_attr];
		if (!(in.attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
			out.attributes[i_attr] = in.attributes[i_attr] + (derivation.attributes[i_attr] * step);
		}
		else{
			out.attributes[i_attr] = in.attributes[i_attr];
		}
	}
	return out;
}	

void viewport_transform(vec4& position, const viewport& vp)
{
	float invw = (eflib::equal<float>(position.w, 0.0f)) ? 1.0f : 1.0f / position.w;
	vec4 pos = position * invw;

	//viewport ±ä»»
	float ox = (vp.x + vp.w) * 0.5f;
	float oy = (vp.y + vp.h) * 0.5f;

	position.x = (float(vp.w) * 0.5f) * pos.x + ox;
	position.y = (float(vp.h) * 0.5f) * -pos.y + oy;
	position.z = (vp.maxz - vp.minz) * 0.5f * pos.z + vp.minz;
	position.w = invw;
}

float compute_area(const vs_output& v0, const vs_output& v1, const vs_output& v2)
{
	return cross_prod2( (v1.position - v0.position).xy(), (v2.position - v0.position).xy() );
}


vs_output::vs_output(
	const eflib::vec4& position, 
	bool front_face,
	const attrib_array_type& attribs,
	const attrib_modifier_array_type& modifiers,
	uint32_t num_used_attrib)
{
	vs_output_ops[num_used_attrib].construct(*this, position, front_face, attribs, modifiers);
}

vs_output::vs_output(const vs_output& rhs)
{
	vs_output_ops[rhs.num_used_attribute].copy(*this, rhs);
}

vs_output& vs_output::operator = (const vs_output& rhs){
	if(&rhs != this){
		vs_output_ops[rhs.num_used_attribute].copy(*this, rhs);
	}
	return *this;
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

END_NS_SOFTART()
