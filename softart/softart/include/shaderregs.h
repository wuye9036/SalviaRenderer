#ifndef SOFTART_SHADERREGS_H
#define SOFTART_SHADERREGS_H

#include "decl.h"
#include "colors.h"
#include "surface.h"
#include "renderer_capacity.h"

#include <eflib/include/math/math.h>

#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6385)
#endif
#include <boost/array.hpp>
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


class vs_input
{
public:
	typedef boost::array<eflib::vec4, vsi_attrib_regcnt> vsinput_attributes_t;

	const eflib::vec4& operator [](size_t i) const;
	eflib::vec4& operator[](size_t i);

	vs_input()
		: num_used_attribute_(0)
	{}
	
	vs_input(vsinput_attributes_t& attrs, uint32_t num_used_attribute)
		: num_used_attribute_(num_used_attribute)
	{
		memcpy(&attributes_[0], &attrs[0], num_used_attribute_ * sizeof(attributes_[0]));
	}

	vs_input(const vs_input& rhs)
		: num_used_attribute_(rhs.num_used_attribute_)
	{
		memcpy(&attributes_[0], &rhs.attributes_[0], num_used_attribute_ * sizeof(attributes_[0]));
	}

	vs_input& operator = (const vs_input& rhs)
	{
		num_used_attribute_ = rhs.num_used_attribute_;
		memcpy(&attributes_[0], &rhs.attributes_[0], num_used_attribute_ * sizeof(attributes_[0]));
	}

protected:
	vsinput_attributes_t attributes_;
	uint32_t num_used_attribute_;
};

class vs_output
{
public:
	enum attrib_modifier_type
	{
		am_linear = 1UL << 0,
		am_centroid = 1UL << 1,
		am_nointerpolation = 1UL << 2,
		am_noperspective = 1UL << 3,
		am_sample = 1UL << 4
	};

public:
	typedef boost::array<eflib::vec4, vso_attrib_regcnt> attrib_array_type;
	typedef boost::array<uint32_t, vso_attrib_regcnt> attrib_modifier_array_type;

	eflib::vec4 position;
	bool front_face;

	attrib_array_type attributes;
	attrib_modifier_array_type attribute_modifiers;

	uint32_t num_used_attribute;

public:
	vs_output()
		: num_used_attribute(0)
	{}
	vs_output(
		const eflib::vec4& position, 
		bool front_face,
		const attrib_array_type& attribs,
		const attrib_modifier_array_type& modifiers,
		uint32_t num_used_attrib);

	//拷贝构造与赋值
	vs_output(const vs_output& rhs);

	vs_output& operator = (const vs_output& rhs);
};

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

	typedef vs_output& (*vs_output_operator_selfadd)(vs_output& lhs, const vs_output& rhs);
	typedef vs_output& (*vs_output_operator_selfsub)(vs_output& lhs, const vs_output& rhs);
	typedef vs_output& (*vs_output_operator_selfmul)(vs_output& lhs, float f);
	typedef vs_output& (*vs_output_operator_selfdiv)(vs_output& lhs, float f);

	typedef vs_output (*vs_output_operator_add)(const vs_output& vso0, const vs_output& vso1);
	typedef vs_output (*vs_output_operator_sub)(const vs_output& vso0, const vs_output& vso1);
	typedef vs_output (*vs_output_operator_mul1)(const vs_output& vso0, float f);
	typedef vs_output (*vs_output_operator_mul2)(float f, const vs_output& vso0);
	typedef vs_output (*vs_output_operator_div)(const vs_output& vso0, float f);

	typedef vs_output (*vs_output_lerp_n)(const vs_output& start, const vs_output& end, float step);

	typedef vs_output& (*vs_output_integral1)(vs_output& inout, const vs_output& derivation);
	typedef vs_output& (*vs_output_integral2)(vs_output& inout, float step, const vs_output& derivation);

	vs_output_construct construct;
	vs_output_copy copy;

	vs_output_project1 project1;
	vs_output_project2 project2;

	vs_output_unproject1 unproject1;
	vs_output_unproject2 unproject2;

	vs_output_operator_selfadd operator_selfadd;
	vs_output_operator_selfsub operator_selfsub;
	vs_output_operator_selfmul operator_selfmul;
	vs_output_operator_selfdiv operator_selfdiv;

	vs_output_operator_add operator_add;
	vs_output_operator_sub operator_sub;
	vs_output_operator_mul1 operator_mul1;
	vs_output_operator_mul2 operator_mul2;
	vs_output_operator_div operator_div;

	vs_output_lerp_n lerp;

	vs_output_integral1 integral1;
	vs_output_integral2 integral2;
};

const vs_output_op& get_vs_output_op(const vs_output& vso);
float compute_area(const vs_output& v0, const vs_output& v1, const vs_output& v2);
void viewport_transform(eflib::vec4& position, const viewport& vp);

//vs_output compute_derivate
struct ps_output
{
	float depth;
	boost::array<eflib::vec4, pso_color_regcnt> color;
	bool front_face;
};

struct backbuffer_pixel_out
{
	backbuffer_pixel_out(std::vector<surface*>& cbuf, surface* dbuf, surface* sbuf)
		:cbuf_(&cbuf), dbuf_(dbuf), sbuf_(sbuf) {
	}

	void set_pos(size_t x, size_t y){
		buf_x_ = x;
		buf_y_ = y;
	}

	color_rgba32f color(size_t regidx, size_t sample) const{
		return (*cbuf_)[regidx]->get_texel(buf_x_, buf_y_, sample);
	}
	float depth(size_t sample) const{
		return dbuf_->get_texel(buf_x_, buf_y_, sample).r;
	}
	int32_t stencil(size_t sample) const{
		return int32_t(sbuf_->get_texel(buf_x_, buf_y_, sample).r);
	}

	void color(size_t regidx, size_t sample, const color_rgba32f& clr){
		(*cbuf_)[regidx]->set_texel(buf_x_, buf_y_, sample, clr);
	}

	void depth(size_t sample, float depth){
		dbuf_->set_texel(buf_x_, buf_y_, sample, color_rgba32f(depth, 0, 0, 0));
	}

	void stencil(size_t sample, int32_t stencil){
		sbuf_->set_texel(buf_x_, buf_y_, sample, color_rgba32f(float(stencil), 0, 0, 0));
	}

private:
	backbuffer_pixel_out(const backbuffer_pixel_out& rhs);
	backbuffer_pixel_out& operator = (const backbuffer_pixel_out& rhs);

	std::vector<surface*>* cbuf_;
	surface* dbuf_;
	surface* sbuf_;
	size_t buf_x_, buf_y_;
};
END_NS_SOFTART()

#endif