#include "../include/shaderregs_op.h"
#include "../include/shader.h"
#include "../include/renderer.h"

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

/**************************************
 *  Vertex Shader Output
 *************************************/
vs_output_impl operator + (const vs_output_impl& vso0, const vs_output_impl& vso1)
{
	vs_output_impl ret_vso;

	ret_vso.position = vso0.position + vso1.position;
	ret_vso.wpos = vso0.wpos + vso1.wpos;
	for(size_t i_color = 0; i_color < vso0.colors.size(); ++i_color){
		ret_vso.colors[i_color]  = vso0.colors[i_color] + vso1.colors[i_color];
	}
	for(size_t i_attr = 0; i_attr < vso0.attributes.size(); ++i_attr){
		ret_vso.attributes[i_attr] = vso0.attributes[i_attr] + vso1.attributes[i_attr];
	}

	return ret_vso;
}

vs_output_impl operator - (const vs_output_impl& vso0, const vs_output_impl& vso1)
{
	vs_output_impl ret_vso;

	ret_vso.position = vso0.position - vso1.position;
	ret_vso.wpos = vso0.wpos - vso1.wpos;
	for(size_t i_color = 0; i_color < vso0.colors.size(); ++i_color){
		ret_vso.colors[i_color]  = vso0.colors[i_color] - vso1.colors[i_color];
	}
	for(size_t i_attr = 0; i_attr < vso0.attributes.size(); ++i_attr){
		ret_vso.attributes[i_attr] = vso0.attributes[i_attr] - vso1.attributes[i_attr];
	}

	return ret_vso;
}

vs_output_impl operator * (const vs_output_impl& vso0, float f)
{
	vs_output_impl ret_vso;
	ret_vso.position = vso0.position * f;
	ret_vso.wpos = vso0.wpos * f;
	for(size_t i_color = 0; i_color < vso0.colors.size(); ++i_color){
		ret_vso.colors[i_color]  = vso0.colors[i_color] * f;
	}
	for(size_t i_attr = 0; i_attr < vso0.attributes.size(); ++i_attr){
		ret_vso.attributes[i_attr] = vso0.attributes[i_attr] * f;
	}
	return ret_vso;
}

vs_output_impl operator * (float f, const vs_output_impl& vso0)
{
	return vso0 * f;
}

vs_output_impl operator / (const vs_output_impl& vso0, float f)
{
	custom_assert(!efl::equal<float>(f, 0.0f), "");
	return vso0 * (1.0f / f);
}

vs_output_impl project(const vs_output_impl& in)
{
	vs_output_impl::attrib_array_type ret_attribs(in.attributes);
	for(size_t i_attr = 0; i_attr < ret_attribs.size(); ++i_attr){
		ret_attribs[i_attr] = in.attributes[i_attr] * in.wpos.w;
	}
	return vs_output_impl(in.position, in.wpos, in.colors, ret_attribs);
}

vs_output_impl& project(vs_output_impl& out, const vs_output_impl& in)
{
	for(size_t i_attr = 0; i_attr < out.attributes.size(); ++i_attr){
		out.attributes[i_attr] = in.attributes[i_attr] * in.wpos.w;
	}
	out.colors = in.colors;
	out.wpos = in.wpos;
	out.position = in.position;
	return out;
}

vs_output_impl unproject(const vs_output_impl& in)
{
	vs_output_impl::attrib_array_type ret_attribs(in.attributes);
	for(size_t i_attr = 0; i_attr < ret_attribs.size(); ++i_attr){
		ret_attribs[i_attr] = in.attributes[i_attr] / in.wpos.w;
	}
	return vs_output_impl(in.position, in.wpos, in.colors, ret_attribs);
}

vs_output_impl& unproject(vs_output_impl& out, const vs_output_impl& in)
{
	for(size_t i_attr = 0; i_attr < out.attributes.size(); ++i_attr){
		out.attributes[i_attr] = in.attributes[i_attr] / in.wpos.w;
	}
	out.colors = in.colors;
	out.wpos = in.wpos;
	out.position = in.position;
	return out;
}

vs_output_impl lerp(const vs_output_impl& start, const vs_output_impl& end, float step)
{
	vs_output_impl out;
	out.position = start.position + (end.position - start.position) * step;
	out.wpos = start.wpos + (end.wpos - start.wpos) * step;
	for(size_t i_color = 0; i_color < out.colors.size(); ++i_color){
		out.colors[i_color] = start.colors[i_color] + (end.colors[i_color] - start.colors[i_color]) * step;
	}
	for(size_t i_attr = 0; i_attr < out.attributes.size(); ++i_attr){
		out.attributes[i_attr] = start.attributes[i_attr] + (end.attributes[i_attr] - start.attributes[i_attr]) * step;
	}
	return out;
}

vs_output_impl& integral(vs_output_impl& inout, float step, const vs_output_impl& derivation)
{
	inout.position += (derivation.position * step);
	inout.wpos += (derivation.wpos * step);
	for(size_t i_color = 0; i_color < inout.colors.size(); ++i_color){
		inout.colors[i_color] += (derivation.colors[i_color] * step);
	}
	for(size_t i_attr = 0; i_attr < inout.attributes.size(); ++i_attr){
		inout.attributes[i_attr] += (derivation.attributes[i_attr] * step);
	}
	return inout;
}

void update_wpos(vs_output_impl& vso, const viewport& vp)
{
	float invw = (efl::equal<float>(vso.position.w, 0.0f)) ? 1.0f : 1.0f / vso.position.w;
	vec4 pos = vso.position * invw;

	//viewport ±ä»»
	float ox = float(vp.x) + float(vp.w) / 2.0f;
	float oy = float(vp.y) + float(vp.h) / 2.0f;

	vso.wpos.x = (float(vp.w) / 2.0f) * pos.x + ox;
	vso.wpos.y = (float(vp.h) / 2.0f) * pos.y + oy;
	vso.wpos.z = (vp.maxz - vp.minz) / 2.0f * pos.z + (vp.maxz + vp.minz) / 2.0f;
	vso.wpos.w = invw;
}

float compute_area(const vs_output_impl& v0, const vs_output_impl& v1, const vs_output_impl& v2)
{
	return cross_prod2( (v1.position - v0.position).xy(), (v2.position - v0.position).xy() );
}

/*****************************************
 *  Vertex Shader
 ****************************************/
void vertex_shader::execute(const vs_input& in, vs_output& out){
	shader_prog(in, out);
}

void blend_shader::execute(backbuffer_pixel_out& out, const backbuffer_pixel_in& in){
	shader_prog(out, in);
}