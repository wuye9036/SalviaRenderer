#include "../include/shaderregs_op.h"
#include "../include/shader.h"
#include "../include/renderer.h"
BEGIN_NS_SOFTART()


using namespace boost;
using namespace efl;

const efl::vec4& vs_input::operator [](size_t i) const
{
	return attributes_[i];
}

efl::vec4& vs_input::operator[](size_t i)
{
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
	custom_assert(!efl::equal<float>(f, 0.0f), "");
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
	return vs_output(vso0) -= vso1;
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
	vs_output::attrib_array_type ret_attribs;
	vs_output::attrib_modifier_array_type ret_attrib_modifiers;
	for(size_t i_attr = 0; i_attr < in.num_used_attribute; ++i_attr){
		ret_attribs[i_attr] = in.attributes[i_attr];
		ret_attrib_modifiers[i_attr] = in.attribute_modifiers[i_attr];
		if (!(in.attribute_modifiers[i_attr] & vs_output::am_noperspective)){
			ret_attribs[i_attr] *= in.position.w;
		}
	}
	return vs_output(in.position, in.front_face, ret_attribs, ret_attrib_modifiers, in.num_used_attribute);
}

vs_output& project(vs_output& out, const vs_output& in)
{
	for(size_t i_attr = 0; i_attr < in.num_used_attribute; ++i_attr){
		out.attributes[i_attr] = in.attributes[i_attr];
		out.attribute_modifiers[i_attr] = in.attribute_modifiers[i_attr];
		if (!(in.attribute_modifiers[i_attr] & vs_output::am_noperspective)){
			out.attributes[i_attr] *= in.position.w;
		}
	}
	out.num_used_attribute = in.num_used_attribute;
	out.position = in.position;
	out.front_face = in.front_face;
	return out;
}

vs_output unproject(const vs_output& in)
{
	vs_output::attrib_array_type ret_attribs;
	vs_output::attrib_modifier_array_type ret_attrib_modifiers;
	float inv_w = 1.0f / in.position.w;
	for(size_t i_attr = 0; i_attr < in.num_used_attribute; ++i_attr){
		ret_attribs[i_attr] = in.attributes[i_attr];
		ret_attrib_modifiers[i_attr] = in.attribute_modifiers[i_attr];
		if (!(in.attribute_modifiers[i_attr] & vs_output::am_noperspective)){
			ret_attribs[i_attr] *= inv_w;
		}
	}
	return vs_output(in.position, in.front_face, ret_attribs, ret_attrib_modifiers, in.num_used_attribute);
}

vs_output& unproject(vs_output& out, const vs_output& in)
{
	float inv_w = 1.0f / in.position.w;
	for(size_t i_attr = 0; i_attr < in.num_used_attribute; ++i_attr){
		out.attributes[i_attr] = in.attributes[i_attr];
		out.attribute_modifiers[i_attr] = in.attribute_modifiers[i_attr];
		if (!(in.attribute_modifiers[i_attr] & vs_output::am_noperspective)){
			out.attributes[i_attr] *= inv_w;
		}
	}
	out.num_used_attribute = in.num_used_attribute;
	out.position = in.position;
	out.front_face = in.front_face;
	return out;
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

vs_output& integral(vs_output& inout, float step, const vs_output& derivation)
{
	assert(inout.num_used_attribute == derivation.num_used_attribute);

	inout.position += (derivation.position * step);
	for(size_t i_attr = 0; i_attr < inout.num_used_attribute; ++i_attr){
		assert(inout.attribute_modifiers[i_attr] == derivation.attribute_modifiers[i_attr]);
		if (!(inout.attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
			inout.attributes[i_attr] += (derivation.attributes[i_attr] * step);
		}
	}
	return inout;
}

vs_output& integral_unproject(vs_output& out, const vs_output& in, float step, const vs_output& derivation)
{
	assert(in.num_used_attribute == derivation.num_used_attribute);

	out.position = in.position + derivation.position * step;

	float inv_w = 1.0f / out.position.w;

	for(size_t i_attr = 0; i_attr < in.num_used_attribute; ++i_attr){
		assert(in.attribute_modifiers[i_attr] == derivation.attribute_modifiers[i_attr]);
		out.attributes[i_attr] = in.attributes[i_attr];
		out.attribute_modifiers[i_attr] = in.attribute_modifiers[i_attr];
		if (!(in.attribute_modifiers[i_attr] & vs_output::am_nointerpolation)){
			out.attributes[i_attr] += derivation.attributes[i_attr] * step;
		}
		if (!(in.attribute_modifiers[i_attr] & vs_output::am_noperspective)){
			out.attributes[i_attr] *= inv_w;
		}
	}
	out.num_used_attribute = in.num_used_attribute;

	return out;
}

void update_wpos(vs_output& vso, const viewport& vp)
{
	float invw = (efl::equal<float>(vso.position.w, 0.0f)) ? 1.0f : 1.0f / vso.position.w;
	vec4 pos = vso.position * invw;

	//viewport ±ä»»
	float ox = (vp.x + vp.w) * 0.5f;
	float oy = (vp.y + vp.h) * 0.5f;

	vso.position.x = (float(vp.w) * 0.5f) * pos.x + ox;
	vso.position.y = (float(vp.h) * 0.5f) * -pos.y + oy;
	vso.position.z = (vp.maxz - vp.minz) * 0.5f * pos.z + vp.minz;
	vso.position.w = invw;
}

float compute_area(const vs_output& v0, const vs_output& v1, const vs_output& v2)
{
	return cross_prod2( (v1.position - v0.position).xy(), (v2.position - v0.position).xy() );
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
