#include "../include/vertex_cache.h"

#include "../include/shader.h"
#include "../include/shaderregs_op.h"
#include "../include/renderer_impl.h"
#include "../include/stream_assembler.h"
#include "../include/geometry_assembler.h"
#include "../include/cpuinfo.h"
#include "../include/thread_pool.h"

#include <boost/ref.hpp>

BEGIN_NS_SOFTART()


const int TRANSFORM_VERTEX_PACKAGE_SIZE = 1;

const size_t invalid_id = 0xffffffff;

default_vertex_cache::default_vertex_cache() : verts_pool_( sizeof(vs_output) )
{
}

void default_vertex_cache::initialize(renderer_impl* psr){
	pparent_ = psr;
}

void default_vertex_cache::reset()
{
	verts_.clear();

	pvs_ = get_weak_handle(pparent_->get_vertex_shader());
	psa_ = &(pparent_->get_geometry_assembler()->get_stream_assembler());
	pvp_ = &(pparent_->get_viewport());
}

void default_vertex_cache::transform_vertex_func(const std::vector<uint32_t>& indices, uint32_t index_count, uint32_t thread_id, uint32_t num_threads)
{
	const uint32_t num_packages = (index_count + TRANSFORM_VERTEX_PACKAGE_SIZE - 1) / TRANSFORM_VERTEX_PACKAGE_SIZE;

	uint32_t local_working_package = thread_id;
	while (local_working_package < num_packages)
	{
		const uint32_t start = local_working_package * TRANSFORM_VERTEX_PACKAGE_SIZE;
		const uint32_t end = min(index_count, start + TRANSFORM_VERTEX_PACKAGE_SIZE);
		for (uint32_t i = start; i < end; ++ i)
		{
			uint32_t id = indices[i];
			used_verts_[id] = i;

			custom_assert(used_verts_[id] == i, "");

			pvs_->execute(psa_->fetch_vertex(id), verts_[i]);
			update_wpos(verts_[i], *pvp_);
		}

		local_working_package += num_threads;
	}
}

void default_vertex_cache::transform_vertices(const std::vector<uint32_t>& indices)
{
	std::vector<uint32_t> unique_indices = indices;
	std::sort(unique_indices.begin(), unique_indices.end());
	unique_indices.erase(std::unique(unique_indices.begin(), unique_indices.end()), unique_indices.end());
	verts_.resize(unique_indices.size());
	used_verts_.assign(psa_->num_vertices(), -1);

#ifdef SOFTART_MULTITHEADING_ENABLED
	uint32_t num_threads = num_cpu_cores();
	for (size_t i = 0; i < num_threads; ++ i)
	{
		global_thread_pool().schedule(boost::bind(&default_vertex_cache::transform_vertex_func, this, boost::ref(unique_indices), static_cast<uint32_t>(unique_indices.size()), i, num_threads));
	}
	global_thread_pool().wait();
#else
	default_vertex_cache::transform_vertex_func(boost::ref(unique_indices), static_cast<uint32_t>(unique_indices.size()), 0, 1);
#endif
}

vs_output& default_vertex_cache::fetch(cache_entry_index id)
{
	static vs_output null_obj;

	if((id > used_verts_.size()) || (-1 == used_verts_[id])){
		custom_assert(false, "");
		return null_obj;
	}

	return verts_[used_verts_[id]];

	//custom_assert(false, "");
	//return null_obj;
}

vs_output& default_vertex_cache::fetch_for_write(cache_entry_index /*id*/)
{
	custom_assert(false, "Deprecated!");
	return verts_[0];
}

vs_output* default_vertex_cache::new_vertex()
{
	return (vs_output*)(verts_pool_.malloc());
}

void default_vertex_cache::delete_vertex(vs_output* const pvert)
{
	bool isfrom = verts_pool_.is_from(pvert);
	custom_assert(isfrom, "");

	if(isfrom)
	{
		pvert->~vs_output();
		verts_pool_.free(pvert);
	}
}

END_NS_SOFTART()
