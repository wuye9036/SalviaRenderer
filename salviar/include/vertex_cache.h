#ifndef SALVIAR_VERTEX_CACHE_H
#define SALVIAR_VERTEX_CACHE_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>

#include <salviar/include/renderer.h>
#include <salviar/include/index_fetcher.h>
#include <eflib/include/memory/atomic.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/pool/pool.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>
#include <utility>

BEGIN_NS_SALVIAR();

class  stream_assembler;
struct render_stages;
struct render_state;

typedef size_t cache_entry_index;

EFLIB_DECLARE_CLASS_SHARED_PTR(vertex_cache);

class vertex_cache
{
public:
	virtual void initialize(render_stages const* stages) = 0;
	virtual void update(render_state const* state) = 0;
	virtual void prepare_vertices() = 0;

	virtual void fetch3(vs_output** v, cache_entry_index id) = 0;
	virtual ~vertex_cache(){}
};

vertex_cache_ptr create_default_vertex_cache();

END_NS_SALVIAR();

#endif