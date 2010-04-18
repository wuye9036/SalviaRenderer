#ifndef SOFTART_SAMPLER_H
#define SOFTART_SAMPLER_H

#include "../include/colors.h"
#include "../include/enums.h"

#include "eflib/include/eflib.h"
#include "softart_fwd.h"
#include "texture.h"
BEGIN_NS_SOFTART()


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
	const texture* ptex_;

	float calc_lod(
		const efl::vec4& attribute, 
		const efl::vec4& size, 
		const efl::vec4& ddx, const efl::vec4& ddy, 
		float inv_x_w, float inv_y_w, float inv_w, float bias) const;

	color_rgba32f sample_surface(
		const surface& surf,
		float x, float y,
		sampler_state ss) const;

	color_rgba32f sample_impl(const texture *tex , float coordx, float coordy, float miplevel) const;

	color_rgba32f sample_impl(const texture *tex , 
		float coordx, float coordy, 
		const efl::vec4& ddx, const efl::vec4& ddy, 
		float inv_x_w, float inv_y_w, float inv_w, float lod_bias) const;

	color_rgba32f sample_2d_impl(const texture *tex , 
		const efl::vec4& coord,
		const efl::vec4& ddx, const efl::vec4& ddy,
		float inv_x_w, float inv_y_w, float inv_w, float lod_bias) const;

public:
	sampler();

	void set_address_mode(sampler_axis axis, address_mode addr_mode);
	void set_filter_type(sampler_state sstate, filter_type filter);
	void set_lod_bias(float bias);
	void set_max_miplevel(int level);
	void set_border_color(const color_rgba32f& border_color);
	void set_texture(const texture* ptex){ptex_ = ptex;}

	color_rgba32f sample(float coordx, float coordy, float miplevel) const;

	color_rgba32f sample(
		float coordx, float coordy, 
		const efl::vec4& ddx, const efl::vec4& ddy, 
		float inv_x_w, float inv_y_w, float inv_w, float lod_bias) const;

	color_rgba32f sample_2d(
		const efl::vec4& coord,
		const efl::vec4& ddx, const efl::vec4& ddy,
		float inv_x_w, float inv_y_w, float inv_w, float lod_bias) const;

	color_rgba32f sample_cube(
		float coordx, float coordy, float coordz,
		float miplevel
		) const;

	color_rgba32f sample_cube(
		const efl::vec4& coord,
		const efl::vec4& ddx,
		const efl::vec4& ddy,
		float inv_x_w, float inv_y_w, float inv_w, float lod_bias
		) const;
};


END_NS_SOFTART()
#endif
