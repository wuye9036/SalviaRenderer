#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>
#include <salviar/include/enums.h>
#include <salviar/include/format.h>

#include <eflib/include/platform/typedefs.h>

BEGIN_NS_SALVIAR();

struct render_state;

class index_fetcher
{
public:
	void update(render_state const* state);
	void fetch_indexes(uint32_t* indexes_of_prim, uint32_t* min_index, uint32_t* max_index, uint32_t prim_id_beg, uint32_t prim_id_end);

private:
	buffer*				index_buffer_;
	format				index_format_;
	primitive_topology	prim_topo_;
	uint32_t			start_addr_;
	uint32_t			base_vert_;
	uint32_t			stride_;
};

END_NS_SALVIAR();
