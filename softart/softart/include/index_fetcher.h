#ifndef SOFTART_INDEX_FETCHER_H
#define SOFTART_INDEX_FETCHER_H

#include "decl.h"
#include "enums.h"

#include <eflib/include/platform/typedefs.h>

#include "softart_fwd.h"

BEGIN_NS_SOFTART();

class index_fetcher
{
public:
	void initialize(h_buffer hbuf, index_type idxtype, primitive_topology primtopo, uint32_t startpos, uint32_t basevert);
	void fetch_indices(uint32_t* prim_indices, uint32_t id);

private:
	h_buffer indexbuf_;
	index_type idxtype_;
	primitive_topology primtopo_;
	uint32_t startpos_;
	uint32_t basevert_;
	uint32_t stride_;
};

END_NS_SOFTART();

#endif