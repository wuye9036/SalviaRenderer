#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/enums.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SALVIAR();

struct raster_desc
{
	fill_mode			fm;
	cull_mode			cm;
	bool				front_ccw;
	int32_t				depth_bias;
	float				depth_bias_clamp;
	float				slope_scaled_depth_bias;
	bool				depth_clip_enable;
	bool				scissor_enable;
	bool				multisample_enable;
	bool				anti_aliased_line_enable;

	raster_desc():
		fm(fill_solid), cm(cull_back),
		front_ccw(false),
		depth_bias(0), depth_bias_clamp(0), slope_scaled_depth_bias(0),
		depth_clip_enable(true), scissor_enable(false),
		multisample_enable(true), anti_aliased_line_enable(false)
	{
	}
};

EFLIB_DECLARE_CLASS_SHARED_PTR(clipper);
EFLIB_DECLARE_CLASS_SHARED_PTR(pixel_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(pixel_shader_unit);

struct clip_context;
struct clip_results;
class  rasterizer;
class  raster_multi_prim_context;
class  vs_output;
struct viewport;

class raster_state
{
public:
	typedef bool (*cull_func)				(float area);

private:
	raster_desc	desc_;
	cull_func	cull_;
	prim_type	prim_;

public:
	raster_state(raster_desc const& desc);

	inline raster_desc const& get_desc() const
	{
		return desc_;
	}

	inline cull_func get_cull_func() const
	{
		return cull_;
	}

	inline bool cull(float area) const
	{
		return cull_(area);
	}
};

END_NS_SALVIAR();
