#ifndef SOFTART_SAMPLER_H
#define SOFTART_SAMPLER_H

#include "../include/colors.h"
#include "../include/enums.h"

#include "eflib/include/eflib.h"

class texture_1d;
class texture_2d;
class surface;

class sampler
{
	filter_type filters_[sampler_state_count];
	address_mode addr_modes_[sampler_axis_count];
	int max_miplevel_;
	float lod_bias_;
	color_rgba32f border_color_;
	mutable const void* ptex_;

	float sampler::calc_lod(
		const efl::vec4& attribute, 
		const efl::vec4& size, 
		const efl::vec4& ddx, const efl::vec4& ddy, 
		float invQ, float bias) const;

	color_rgba32f sample_surface(
		const surface& surf,
		float x, float y,
		sampler_state ss) const;

public:
	sampler();

	void set_address_mode(sampler_axis axis, address_mode addr_mode);
	void set_filter_type(sampler_state sstate, filter_type filter);
	void set_lod_bias(float bias);
	void set_max_miplevel(int level);
	void set_border_color(const color_rgba32f& border_color);
	void set_texture(const void* ptex){ptex_ = ptex;}

	color_rgba32f sample(float coordx, float coordy, float miplevel) const;

	color_rgba32f sample(
		float coordx, float coordy, 
		const efl::vec4& ddx, const efl::vec4& ddy, 
		float invQ, float lod_bias) const;

	color_rgba32f sample_2d(
		const efl::vec4& coord,
		const efl::vec4& ddx, const efl::vec4& ddy,
		float invQ, float lod_bias) const;

	color_rgba32f sample_cube(
		float coordx, float coordy, float coordz,
		float miplevel
		) const;

	color_rgba32f sample_cube(
		const efl::vec4& coord,
		const efl::vec4& ddx,
		const efl::vec4& ddy,
		float invQ, float lod_bias
		) const;
};

#endif