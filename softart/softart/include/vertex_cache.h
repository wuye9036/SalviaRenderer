#ifndef SOFTART_VERTEX_CACHE_H
#define SOFTART_VERTEX_CACHE_H

#include "decl.h"

#include "eflib/include/eflib.h"

#include <vector>
#include <utility>

#include "renderer.h"
#include "render_stage.h"
#include "index_fetcher.h"

#include <boost/pool/pool.hpp>

#include "softart_fwd.h"
BEGIN_NS_SOFTART()


class stream_assembler;

//当顶点直接来自于input vertex时，则只有low标号起作用；如果顶点为裁切后的顶点，则hi id也会起作用
typedef size_t cache_entry_index;

class vertex_cache : public render_stage
{
public:
	virtual void reset(const h_buffer& hbuf, index_type idxtype, primitive_topology primtopo, uint32_t startpos, uint32_t basevert) = 0;

	virtual result set_input_layout(const input_layout_decl& layout) = 0;
	virtual result set_stream(stream_index sidx, h_buffer hbuf) = 0;

	virtual void transform_vertices(uint32_t prim_count) = 0;

	virtual vs_output& fetch(cache_entry_index id) = 0;
	virtual vs_output& fetch_for_write(cache_entry_index id) = 0;

	virtual vs_output* new_vertex() = 0;
	virtual void delete_vertex(vs_output* const pvert) = 0;
};

class default_vertex_cache : public vertex_cache
{
public:
	default_vertex_cache();

	void initialize(renderer_impl* psr);

	void reset(const h_buffer& hbuf, index_type idxtype, primitive_topology primtopo, uint32_t startpos, uint32_t basevert);

	result set_input_layout(const input_layout_decl& layout);
	result set_stream(stream_index sidx, h_buffer hbuf);

	void transform_vertices(uint32_t prim_count);

	vs_output& fetch(cache_entry_index id);
	vs_output& fetch_for_write(cache_entry_index id);

	vs_output* new_vertex();
	void delete_vertex(vs_output* const pvert);

private:
	void transform_vertex_func(const std::vector<uint32_t>& indices, uint32_t index_count, uint32_t thread_id, uint32_t num_threads);
	void generate_indices_func(std::vector<uint32_t>& indices, uint32_t prim_count, uint32_t stride, uint32_t thread_id, uint32_t num_threads);

private:
	vertex_shader* pvs_;
	h_stream_assembler hsa_;

	primitive_topology primtopo_;
	index_fetcher ind_fetcher_;
	std::vector<uint32_t> indices_;

	std::vector<vs_output> verts_;
	std::vector<int32_t> used_verts_;

	boost::pool<> verts_pool_;
	const viewport* pvp_;
};

END_NS_SOFTART()

#endif