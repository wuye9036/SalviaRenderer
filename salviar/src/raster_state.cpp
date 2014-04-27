#include <salviar/include/raster_state.h>
#include <salviar/include/clipper.h>
#include <salviar/include/shader_regs.h>
#include <salviar/include/rasterizer.h>

using boost::function;

BEGIN_NS_SALVIAR();

bool cull_mode_none(float /*area*/)
{
	return false;
}

bool cull_mode_ccw(float area)
{
	return area <= 0;
}

bool cull_mode_cw(float area)
{
	return area >= 0;
}

raster_state::raster_state(const raster_desc& desc)
	: desc_(desc)
{
	switch (desc.cm)
	{
	case cull_none:
		cull_ = cull_mode_none;
		break;

	case cull_front:
		cull_ = desc.front_ccw ? cull_mode_ccw : cull_mode_cw;
		break;

	case cull_back:
		cull_ = desc.front_ccw ? cull_mode_cw : cull_mode_ccw;
		break;

	default:
		EFLIB_ASSERT_UNEXPECTED();
		break;
	}
}

END_NS_SALVIAR();