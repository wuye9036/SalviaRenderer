#ifndef SOFTART_GEOMETRY_ASSEMBLER_H
#define SOFTART_GEOMETRY_ASSEMBLER_H

#include "render_stage.h"
#include "stream_assembler.h"
#include "vertex_cache.h"

#include "renderer_capacity.h"
#include "decl.h"
#include "enums.h"
#include "handles.h"

#include "atomic.h"
#include "lockfree_queue.h"

#include <boost/array.hpp>
#include <boost/thread.hpp>

#include "boost/threadpool.hpp"

#include <vector>

class geometry_assembler : public render_stage
{
	h_buffer indexbuf_;
	std::vector<h_buffer> streams_;
	std::vector<input_element_decl> layout_;
	primitive_topology primtopo_;
	index_type	idxtype_;

	template<class T>
		void get_vert_range(size_t startpos, size_t count, int basevert, size_t& min_vert, size_t& max_vert);

	vs_output* transform_vertex(size_t startpos, size_t count);

	template<class T>
		void draw_index_impl(size_t startpos, size_t prim_count, int basevert);

	template<class T>
		void dispatch_primitive_impl(std::vector<lockfree_queue<uint32_t> >& tiles, std::vector<T> const & indices, atomic<int32_t>& working_prim, int32_t prim_count);

	template<class T>
		void rasterize_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, const std::vector<T>& indices, atomic<int32_t>& working_tile);

	template<class T>
		void generate_line_list_indices_impl(std::vector<T>& indices, T* pidx, atomic<int32_t>& working_prim, int32_t prim_count);
	template<class T>
		void generate_line_strip_indices_impl(std::vector<T>& indices, T* pidx, atomic<int32_t>& working_prim, int32_t prim_count);
	template<class T>
		void generate_triangle_list_indices_impl(std::vector<T>& indices, T* pidx, atomic<int32_t>& working_prim, int32_t prim_count);
	template<class T>
		void generate_triangle_strip_indices_impl(std::vector<T>& indices, T* pidx, atomic<int32_t>& working_prim, int32_t prim_count);

	stream_assembler sa_;
	default_vertex_cache dvc_;
	int num_tiles_x_, num_tiles_y_;

	int num_threads_;
	boost::threadpool::pool tp_;

public:
	//
	//Inherited
	void initialize(renderer_impl* pparent);

	//构造函数
	geometry_assembler();

	//设置函数
	void set_primitive_topology(primitive_topology prim_topo);
	void set_stream(size_t index, h_buffer hbuf);
	void set_index_buffer(h_buffer hbuf, index_type idxtype);
	void set_input_layout(const std::vector<input_element_decl>& elem_decl);

	//绘制函数
	void draw(size_t startpos, size_t count);
	void draw_index(size_t startpos, size_t prim_count, int basevert);

	stream_assembler& get_stream_assembler(){
		return sa_;
	}
};

#endif