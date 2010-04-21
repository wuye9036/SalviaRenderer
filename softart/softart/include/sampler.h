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

struct sampler_desc {
    filter_type min_filter;
	filter_type mag_filter;
	filter_type mip_filter;
    address_mode addr_mode_u;
    address_mode addr_mode_v;
    address_mode addr_mode_w;
    float mip_lod_bias;
    uint32_t max_anisotropy;
    compare_function comparison_func;
    color_rgba32f border_color;
    float min_lod;
    float max_lod;
};

class sampler
{
public:
	typedef void (*coord_calculator_op_type)(int& low, int& up, float& frac, float coord, int size);
	typedef color_rgba32f (*filter_op_type)(const surface& surf, float x, float y, const coord_calculator_op_type* addr_mode0, const coord_calculator_op_type* addr_mode1, const color_rgba32f& border_color);

private:
	sampler_desc desc_;
	const texture* ptex_;
	filter_op_type filters_[sampler_state_count];
	const coord_calculator_op_type* addr_modes_[sampler_axis_count];

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
	explicit sampler(const sampler_desc& desc);

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
