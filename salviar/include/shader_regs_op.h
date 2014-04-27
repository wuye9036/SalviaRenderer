#pragma once

#include <salviar/include/salviar_forward.h>
#include <eflib/include/math/vector.h>

BEGIN_NS_SALVIAR();

struct viewport;
class  vs_output;

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
    vs_output_functions::step_2d_unproj	step_2d_unproj_pos;
    vs_output_functions::step_2d_unproj	step_2d_unproj_attr;

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

END_NS_SALVIAR();
