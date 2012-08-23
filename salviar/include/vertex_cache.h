#ifndef SALVIAR_VERTEX_CACHE_H
#define SALVIAR_VERTEX_CACHE_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>

#include <salviar/include/renderer.h>
#include <salviar/include/render_stage.h>
#include <salviar/include/index_fetcher.h>
#include <eflib/include/memory/atomic.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/pool/pool.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>
#include <utility>

BEGIN_NS_SALVIAR();

class stream_assembler;

typedef size_t cache_entry_index;

class vertex_cache : public render_stage
{
public:
	virtual void reset(const h_buffer& hbuf, format index_fmt, primitive_topology primtopo, uint32_t startpos, uint32_t basevert) = 0;

	virtual result set_input_layout( h_input_layout const& layout) = 0;

	virtual result set_vertex_buffers(
		size_t starts_slot,
		size_t buffers_count, h_buffer const* buffers,
		size_t const* strides, size_t const* offsets
		) = 0;

	virtual void transform_vertices(uint32_t prim_count) = 0;

	virtual vs_output& fetch(cache_entry_index id) = 0;

	virtual vs_output* new_vertex() = 0;
	virtual void delete_vertex(vs_output* const pvert) = 0;
};

class default_vertex_cache : public vertex_cache
{
public:
	default_vertex_cache();

	void initialize(renderer_impl* psr);

	void reset(const h_buffer& hbuf, format index_fmt, primitive_topology primtopo, uint32_t startpos, uint32_t basevert);

	result set_input_layout(const h_input_layout& layout);
	result set_vertex_buffers(
		size_t starts_slot,
		size_t buffers_count, h_buffer const* buffers,
		size_t const* strides, size_t const* offsets
		);

	void transform_vertices(uint32_t prim_count);

	vs_output& fetch(cache_entry_index id);

	vs_output* new_vertex();
	void delete_vertex(vs_output* const pvert);

private:
	void generate_indices_func(std::vector<uint32_t>& indices, int32_t prim_count, uint32_t stride, eflib::atomic<int32_t>& working_package, int32_t package_size);
	void transform_vertex_func(const std::vector<uint32_t>& indices, int32_t index_count, eflib::atomic<int32_t>& working_package, int32_t package_size);
	void transform_vertex_by_shader( const std::vector<uint32_t>& indices, int32_t index_count, eflib::atomic<int32_t>& working_package, int32_t package_size );
private:
	vertex_shader* pvs_;
	h_stream_assembler hsa_;

	primitive_topology primtopo_;
	index_fetcher ind_fetcher_;
	std::vector<uint32_t> indices_;

	boost::shared_array<vs_output> verts_;
	std::vector<int32_t> used_verts_;

	boost::pool<> verts_pool_;
	const viewport* pvp_;
};


END_NS_SALVIAR()

#endif