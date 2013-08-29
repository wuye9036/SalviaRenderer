#pragma once

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

struct viewport;

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
		eflib::vec4, MAX_VS_INPUT_ATTRS > attribute_array;
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
		eflib::vec4, MAX_VS_OUTPUT_ATTRS+1 > register_array;
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

namespace vs_output_functions
{
	using eflib::vec4;

	typedef vs_output& (*construct)		(vs_output& out, vec4 const& position, bool front_face, vec4 const* attrs);
	typedef vs_output& (*copy)			(vs_output& out, const vs_output& in);
	
	typedef vs_output& (*project)		(vs_output& out, const vs_output& in);
	typedef vs_output& (*unproject)		(vs_output& out, const vs_output& in);

	typedef vs_output& (*self_add)		(vs_output& lhs, const vs_output& rhs);
	typedef vs_output& (*self_sub)		(vs_output& lhs, const vs_output& rhs);
	typedef vs_output& (*self_mul)		(vs_output& lhs, float f);
	typedef vs_output& (*self_div)		(vs_output& lhs, float f);

	typedef vs_output& (*add)			(vs_output& out, const vs_output& vso0, const vs_output& vso1);
	typedef vs_output& (*sub)			(vs_output& out, const vs_output& vso0, const vs_output& vso1);
	typedef vs_output& (*mul)			(vs_output& out, const vs_output& vso0, float f);
	typedef vs_output& (*div)			(vs_output& out, const vs_output& vso0, float f);

	typedef vs_output& (*lerp)			(vs_output& out, const vs_output& start, const vs_output& end, float step);
	typedef vs_output& (*step_unproj)	(vs_output& out, vs_output const& start, vs_output const& derivation);
	typedef vs_output& (*step_2d_unproj)(
		vs_output& out, vs_output const& start,
		float step0, vs_output const& derivation0,
		float step1, vs_output const& derivation1);

	typedef vs_output& (*step1)			(vs_output& out, const vs_output& in, const vs_output& derivation);
	typedef vs_output& (*step_1d)		(vs_output& out, const vs_output& in, float step, const vs_output& derivation);
	typedef vs_output& (*step_2d)		(
		vs_output& out, const vs_output& in,
		float step0, vs_output const& derivation0,
		float step1, vs_output const& derivation1);
	typedef vs_output& (*self_step1)	(vs_output& inout, const vs_output& derivation);
	typedef vs_output& (*self_step_1d)	(vs_output& inout, float step, const vs_output& derivation);
}

struct vs_output_op
{
	vs_output_functions::construct		construct;
	vs_output_functions::copy			copy;

	vs_output_functions::project		project;
	vs_output_functions::unproject		unproject;
	
	vs_output_functions::self_add		self_add;
	vs_output_functions::self_sub		self_sub;
	vs_output_functions::self_mul		self_mul;
	vs_output_functions::self_div		self_div;

	vs_output_functions::add			add;
	vs_output_functions::sub			sub;
	vs_output_functions::mul			mul;
	vs_output_functions::div			div;

	vs_output_functions::lerp			lerp;
	vs_output_functions::step_unproj	step_unproj;
	vs_output_functions::step_2d_unproj	step_2d_unproj;
	vs_output_functions::step1			step1;
	vs_output_functions::step_1d		step_1d;
	vs_output_functions::step_2d		step_2d;
	vs_output_functions::self_step1		self_step1;
	vs_output_functions::self_step_1d	self_step_1d;

	typedef boost::array<uint32_t, MAX_VS_OUTPUT_ATTRS> interpolation_modifier_array;
	interpolation_modifier_array		attribute_modifiers;
};

vs_input_op& get_vs_input_op(uint32_t n);
vs_output_op& get_vs_output_op(uint32_t n);
float compute_area(const vs_output& v0, const vs_output& v1, const vs_output& v2);
void viewport_transform(eflib::vec4& position, viewport const& vp);

//vs_output compute_derivate
struct ps_output
{
	float		depth;
	bool		front_face;
	uint32_t	coverage;
	boost::array<eflib::vec4, MAX_RENDER_TARGETS> color;
};

struct pixel_accessor
{
	pixel_accessor(surface** const& color_buffers, surface* ds_buffer)
    {
        color_buffers_ = color_buffers;
        ds_buffer_ = ds_buffer;
	}

	void set_pos(size_t x, size_t y)
    {
		x_ = x;
		y_ = y;
	}

	color_rgba32f color(size_t target_index, size_t sample_index) const
	{
		return color_buffers_[target_index]->get_texel(x_, y_, sample_index);
	}

	void color(size_t register_index, size_t sample, const color_rgba32f& clr)
	{
        color_buffers_[register_index]->set_texel(x_, y_, sample, clr);
	}

    void* depth_stencil_address(size_t sample) const
	{
        return ds_buffer_->texel_address(x_, y_, sample);
	}

private:
	pixel_accessor(const pixel_accessor& rhs);
	pixel_accessor& operator = (const pixel_accessor& rhs);

    surface**   color_buffers_;
	surface*    ds_buffer_;
	size_t      x_, y_;
};

END_NS_SALVIAR();
