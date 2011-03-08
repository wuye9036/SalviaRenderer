#include "../include/vertex_cache.h"
#include "../include/stream_assembler.h"

#include <eflib/include/platform/config.h>

#include "../include/shader.h"
#include "../include/shaderregs_op.h"
#include "../include/renderer_impl.h"
#include "../include/stream_assembler.h"
#include "../include/thread_pool.h"

#include <eflib/include/platform/cpuinfo.h>

#include <boost/ref.hpp>

using eflib::num_available_threads;

BEGIN_NS_SOFTART()

const int GENERATE_INDICES_PACKAGE_SIZE = 8;
const int TRANSFORM_VERTEX_PACKAGE_SIZE = 8;

const size_t invalid_id = 0xffffffff;

default_vertex_cache::default_vertex_cache() : verts_pool_( sizeof(vs_output) )
{
	hsa_.reset(new stream_assembler);
}

void default_vertex_cache::initialize(renderer_impl* psr){
	pparent_ = psr;
}

void default_vertex_cache::reset(const h_buffer& hbuf, index_type idxtype, primitive_topology primtopo, uint32_t startpos, uint32_t basevert)
{
	verts_.reset();

	pvs_ = get_weak_handle(pparent_->get_vertex_shader());
	pvp_ = &(pparent_->get_viewport());

	primtopo_ = primtopo;
	ind_fetcher_.initialize(hbuf, idxtype, primtopo, startpos, basevert);
}

void default_vertex_cache::generate_indices_func(std::vector<uint32_t>& indices, int32_t prim_count, uint32_t stride, atomic<int32_t>& working_package, int32_t package_size)
{
	const int32_t num_packages = (prim_count + package_size - 1) / package_size;

	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages){
		const int32_t start = local_working_package * package_size;
		const int32_t end = std::min(prim_count, start + package_size);
		for (int32_t i = start; i < end; ++ i){
			ind_fetcher_.fetch_indices(&indices[i * stride], i);
		}

		local_working_package = working_package ++;
	}
}

void default_vertex_cache::transform_vertex_func(const std::vector<uint32_t>& indices, int32_t index_count, atomic<int32_t>& working_package, int32_t package_size)
{
	const int32_t num_packages = (index_count + package_size - 1) / package_size;

	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages)
	{
		const int32_t start = local_working_package * package_size;
		const int32_t end = std::min(index_count, start + package_size);
		for (int32_t i = start; i < end; ++ i){
			uint32_t id = indices[i];
			used_verts_[id] = i;

			vs_input vertex;
			hsa_->fetch_vertex(vertex, id);
			pvs_->execute(vertex, verts_[i]);
		}

		local_working_package = working_package ++;
	}
}

void default_vertex_cache::transform_vertices(uint32_t prim_count)
{
	uint32_t prim_size = 0;
	switch(primtopo_)
	{
	case primitive_line_list:
	case primitive_line_strip:
		prim_size = 2;
		break;

	case primitive_triangle_list:
	case primitive_triangle_strip:
		prim_size = 3;
		break;
	}

	indices_.resize(prim_count * prim_size);

	atomic<int32_t> working_package(0);
	size_t num_threads = num_available_threads( );

	for (size_t i = 0; i < num_threads - 1; ++ i){
		global_thread_pool().schedule(boost::bind(&default_vertex_cache::generate_indices_func, this, boost::ref(indices_), static_cast<int32_t>(prim_count), prim_size, boost::ref(working_package), GENERATE_INDICES_PACKAGE_SIZE));
	}
	generate_indices_func(boost::ref(indices_), static_cast<int32_t>(prim_count), prim_size, boost::ref(working_package), GENERATE_INDICES_PACKAGE_SIZE);
	global_thread_pool().wait();

	std::vector<uint32_t> unique_indices = indices_;
	std::sort(unique_indices.begin(), unique_indices.end());
	unique_indices.erase(std::unique(unique_indices.begin(), unique_indices.end()), unique_indices.end());
	verts_.reset(new vs_output[unique_indices.size()]);
	used_verts_.resize(hsa_->num_vertices());

	working_package = 0;
	for (size_t i = 0; i < num_threads - 1; ++ i)
	{
		global_thread_pool().schedule(boost::bind(&default_vertex_cache::transform_vertex_func, this, boost::ref(unique_indices), static_cast<int32_t>(unique_indices.size()), boost::ref(working_package), TRANSFORM_VERTEX_PACKAGE_SIZE));
	}
	transform_vertex_func(boost::ref(unique_indices), static_cast<int32_t>(unique_indices.size()), boost::ref(working_package), TRANSFORM_VERTEX_PACKAGE_SIZE);
	global_thread_pool().wait();
}

vs_output& default_vertex_cache::fetch(cache_entry_index id)
{
	static vs_output null_obj;

	id = indices_[id];

	if((id > used_verts_.size()) || (-1 == used_verts_[id])){
		EFLIB_ASSERT(false, "");
		return null_obj;
	}

	return verts_[used_verts_[id]];

	//EFLIB_ASSERT(false, "");
	//return null_obj;
}

vs_output& default_vertex_cache::fetch_for_write(cache_entry_index /*id*/)
{
	EFLIB_ASSERT(false, "Deprecated!");
	return verts_[0];
}

vs_output* default_vertex_cache::new_vertex()
{
	return (vs_output*)(verts_pool_.malloc());
}

void default_vertex_cache::delete_vertex(vs_output* const pvert)
{
	bool isfrom = verts_pool_.is_from(pvert);
	EFLIB_ASSERT(isfrom, "");

	if(isfrom)
	{
		pvert->~vs_output();
		verts_pool_.free(pvert);
	}
}

result default_vertex_cache::set_input_layout(const input_layout_decl& layout)
{
	//layout_ 只能到运行期检测了...
	hsa_->set_input_layout(layout);
	return result::ok;
}

result default_vertex_cache::set_stream(stream_index sidx, h_buffer hbuf)
{
	hsa_->set_stream(stream_index(sidx), hbuf);
	return result::ok;
}

END_NS_SOFTART()
