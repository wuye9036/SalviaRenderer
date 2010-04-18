#ifndef SOFTART_CLIPPER_H
#define SOFTART_CLIPPER_H

#include "decl.h"
#include "eflib/include/eflib.h"

#include <boost/array.hpp>

#include <vector>
#include "softart_fwd.h"
BEGIN_NS_SOFTART()

const size_t plane_num = 10;
const size_t default_plane_num = 6;

class clipper
{
	boost::array<efl::vec4, plane_num>				planes_;
	boost::array<bool, plane_num>					planes_enable_;

public:
	clipper();

	void set_clip_plane(const efl::vec4& plane, size_t idx);
	void set_clip_plane_enable(bool enable, size_t idx);

	void clip(std::vector<vs_output> &out_clipped_verts, const viewport& vp, const vs_output& v0, const vs_output& v1, const vs_output& v2);

};

END_NS_SOFTART()

#endif