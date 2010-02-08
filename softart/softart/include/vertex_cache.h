#ifndef SOFTART_VERTEX_CACHE_H
#define SOFTART_VERTEX_CACHE_H

#include "decl.h"

#include "eflib/include/eflib.h"

#include <vector>
#include <utility>

#include "render_stage.h"

#include <boost/pool/pool.hpp>

#include "atomic.h"

class stream_assembler;

//当顶点直接来自于input vertex时，则只有low标号起作用；如果顶点为裁切后的顶点，则hi id也会起作用
typedef size_t cache_entry_index;

class vertex_cache : public render_stage
{
public:
	virtual void set_vert_range(size_t minvert, size_t maxvert) = 0;
	virtual void reset() = 0;

	virtual void transform_vertices(const std::vector<uint16_t>& indices) = 0;
	virtual void transform_vertices(const std::vector<uint32_t>& indices) = 0;

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

	void set_vert_range(size_t minvert, size_t maxvert);
	void reset();

	void transform_vertices(const std::vector<uint16_t>& indices);
	void transform_vertices(const std::vector<uint32_t>& indices);

	vs_output& fetch(cache_entry_index id);
	vs_output& fetch_for_write(cache_entry_index id);

	vs_output* new_vertex();
	void delete_vertex(vs_output* const pvert);

private:
	template<class T>
		void transform_vertex_impl(const std::vector<T>& indices, atomic<int32_t>& working_index, int32_t index_count);

private:
	size_t vert_base_;
	vertex_shader* pvs_;
	stream_assembler* psa_;

	std::vector<vs_output> verts_;

	boost::pool<> verts_pool_;
	const viewport* pvp_;
};

#endif