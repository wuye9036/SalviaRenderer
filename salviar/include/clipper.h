#ifndef SOFTART_CLIPPER_H
#define SOFTART_CLIPPER_H

#include "decl.h"
#include <salviar/include/salviar_forward.h>

#include <eflib/include/math/math.h>
#include <boost/array.hpp>
#include <vector>

BEGIN_NS_SALVIAR()

const size_t plane_num = 6;

class clipper
{
	boost::array<eflib::vec4, plane_num>				planes_;
	boost::array<bool, plane_num>					planes_enable_;

public:
	clipper();

	void set_clip_plane_enable(bool enable, size_t idx);

	void clip(vs_output* out_clipped_verts, uint32_t& num_out_clipped_verts, const viewport& vp, const vs_output& v0, const vs_output& v1, const vs_output_op& vs_output_ops) const;
	void clip(vs_output* out_clipped_verts, uint32_t& num_out_clipped_verts, const viewport& vp, const vs_output& v0, const vs_output& v1, const vs_output& v2, const vs_output_op& vs_output_ops) const;

};

END_NS_SALVIAR()

#endif