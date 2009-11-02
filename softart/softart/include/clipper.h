#ifndef SOFTART_CLIPPER_H
#define SOFTART_CLIPPER_H

#include "eflib/include/eflib.h"

#include <boost/array.hpp>
#include <boost/pool/pool.hpp>

#include <vector>

struct viewport;
class vs_output_impl;

//template<class Pool, size_t plane_num, size_t clipped_vert_list_size>
const size_t plane_num = 10;

class clipper
{ 
	const viewport*												pvp_;
	efl::recycled_pool<32, efl::pool::unsafe_debug>	pool_;
	std::vector<const vs_output_impl*>							clipped_verts_[2];
	std::vector<bool>											is_vert_from_pool_[2];
	boost::array<efl::vec4, plane_num>					planes_;
	boost::array<bool, plane_num>							planes_enable_;
	size_t																last_stage;

public:
	clipper();

	void set_viewport(const viewport& vp);
	void set_clip_plane(const efl::vec4& plane, size_t idx);
	void set_clip_plane_enable(bool enable, size_t idx);

	const std::vector<const vs_output_impl*>& clip(const vs_output_impl& v0, const vs_output_impl& v1, const vs_output_impl& v2);

};


#endif