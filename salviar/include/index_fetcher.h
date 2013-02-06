#ifndef SALVIAR_INDEX_FETCHER_H
#define SALVIAR_INDEX_FETCHER_H

#include "decl.h"
#include "enums.h"

#include <eflib/include/platform/typedefs.h>

#include <salviar/include/salviar_forward.h>
#include <salviar/include/format.h>
BEGIN_NS_SALVIAR();

class index_fetcher
{
public:
	void initialize(h_buffer hbuf, format index_fmt, primitive_topology primtopo, uint32_t startpos, uint32_t basevert);
	void fetch_indices(uint32_t* prim_indices, uint32_t id);

private:
	h_buffer index_buffer_;
	format index_format_;
	primitive_topology primtopo_;
	uint32_t startpos_;
	uint32_t basevert_;
	uint32_t stride_;
};

END_NS_SALVIAR();

#endif