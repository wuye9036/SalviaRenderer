#pragma once

#ifndef SALVIAR_SHADERREGS_H
#define SALVIAR_SHADERREGS_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>
#include <salviar/include/colors.h>
#include <salviar/include/surface.h>
#include <salviar/include/renderer_capacity.h>

#include <eflib/include/math/math.h>
#include <eflib/include/memory/allocator.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/array.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

class vs_input
{
public:
	vs_input()
	{}
	
	eflib::vec4& attribute(size_t index)
	{
		return attributes_[index];
	}
	
	eflib::vec4 const& attribute(size_t index) const
	{
		return attributes_[index];
	}

private:
	typedef boost::array<
		eflib::vec4, vsi_attribute_count > attribute_array;
	attribute_array attributes_;

	vs_input(const vs_input& rhs);
	vs_input& operator=(const vs_input& rhs);
};

#include <eflib/include/platform/disable_warnings.h>
class ALIGN16 vs_output
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
	eflib::vec4& position()
	{
		return registers_[0];
	}
	
	eflib::vec4 const& position() const
	{
		return registers_[0];
	}

	eflib::vec4* attribute_data()
	{
		return registers_.data() + 1;
	}

	eflib::vec4 const* attribute_data() const
	{
		return registers_.data() + 1;
	}

	eflib::vec4* raw_data()
	{
		return registers_.data();
	}

	eflib::vec4 const* raw_data() const
	{
		return registers_.data();
	}

	eflib::vec4 const& attribute(size_t index) const
	{
		return attribute_data()[index];
	}

	eflib::vec4& attribute(size_t index)
	{
		return attribute_data()[index];
	}

	bool front_face() const
	{
		return front_face_;
	}

	void front_face(bool v)
	{
		front_face_ = v;
	}

	vs_output()
	{}

private:
	typedef boost::array<
		eflib::vec4, vso_attribute_count+1 > register_array;
	register_array registers_;

	bool front_face_;
	
	vs_output(const vs_output& rhs);
	vs_output& operator=(const vs_output& rhs);
};
#include <eflib/include/platform/enable_warnings.h>

struct vs_input_op
{
	typedef vs_input& (*vs_input_construct)(vs_input& out,
		eflib::vec4 const* attrs);
	typedef vs_input& (*vs_input_copy)(vs_input& out, const vs_input& in);

	vs_input_construct construct;
	vs_input_copy copy;
};

struct vs_output_op
{
	typedef vs_output& (*vs_output_construct)(
		vs_output& out,
		eflib::vec4 const& position, bool front_face,
		eflib::vec4 const* attrs
		);
	typedef vs_output& (*vs_output_copy)(vs_output& out, const vs_output& in);
	
	typedef vs_output& (*vs_output_project)(vs_output& out, const vs_output& in);
	typedef vs_output& (*vs_output_unproject)(vs_output& out, const vs_output& in);

	typedef vs_output& (*vs_output_operator_selfadd)(vs_output& lhs, const vs_output& rhs);
	typedef vs_output& (*vs_output_operator_selfsub)(vs_output& lhs, const vs_output& rhs);
	typedef vs_output& (*vs_output_operator_selfmul)(vs_output& lhs, float f);
	typedef vs_output& (*vs_output_operator_selfdiv)(vs_output& lhs, float f);

	typedef vs_output& (*vs_output_operator_add)(vs_output& out, const vs_output& vso0, const vs_output& vso1);
	typedef vs_output& (*vs_output_operator_sub)(vs_output& out, const vs_output& vso0, const vs_output& vso1);
	typedef vs_output& (*vs_output_operator_mul)(vs_output& out, const vs_output& vso0, float f);
	typedef vs_output& (*vs_output_operator_div)(vs_output& out, const vs_output& vso0, float f);

	typedef vs_output& (*vs_output_lerp_n)(vs_output& out, const vs_output& start, const vs_output& end, float step);

	typedef vs_output& (*vs_output_integral1)(vs_output& out, const vs_output& in, const vs_output& derivation);
	typedef vs_output& (*vs_output_integral2)(vs_output& out, const vs_output& in, float step, const vs_output& derivation);
	typedef vs_output& (*vs_output_selfintegral1)(vs_output& inout, const vs_output& derivation);
	typedef vs_output& (*vs_output_selfintegral2)(vs_output& inout, float step, const vs_output& derivation);

	typedef boost::array<uint32_t, vso_attribute_count> interpolation_modifier_array;


	vs_output_construct construct;
	vs_output_copy copy;

	vs_output_project project;
	vs_output_unproject unproject;

	vs_output_operator_selfadd operator_selfadd;
	vs_output_operator_selfsub operator_selfsub;
	vs_output_operator_selfmul operator_selfmul;
	vs_output_operator_selfdiv operator_selfdiv;

	vs_output_operator_add operator_add;
	vs_output_operator_sub operator_sub;
	vs_output_operator_mul operator_mul;
	vs_output_operator_div operator_div;

	vs_output_lerp_n lerp;

	vs_output_integral1 integral1;
	vs_output_integral2 integral2;
	vs_output_selfintegral1 selfintegral1;
	vs_output_selfintegral2 selfintegral2;

	interpolation_modifier_array attribute_modifiers;
};

vs_input_op& get_vs_input_op(uint32_t n);
vs_output_op& get_vs_output_op(uint32_t n);
float compute_area(const vs_output& v0, const vs_output& v1, const vs_output& v2);
void viewport_transform(eflib::vec4& position, const viewport& vp);

//vs_output compute_derivate
struct ps_output
{
	float depth;
	boost::array<eflib::vec4, pso_color_regcnt> color;
	bool front_face;
	uint32_t coverage;
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
END_NS_SALVIAR()

#endif