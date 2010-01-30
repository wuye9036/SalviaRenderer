#include "../include/vertex_cache.h"

#include "../include/shader.h"
#include "../include/shaderregs_op.h"
#include "../include/renderer_impl.h"
#include "../include/stream_assembler.h"
#include "../include/geometry_assembler.h"
#include "../include/cpuinfo.h"

const size_t invalid_id = 0xffffffff;

default_vertex_cache::default_vertex_cache() : verts_pool_( sizeof(vs_output) )
{
}

void default_vertex_cache::initialize(renderer_impl* psr){
	pparent_ = psr;
}

void default_vertex_cache::set_vert_range(size_t minvert, size_t /*maxvert*/)
{
	vert_base_ = minvert;
}

void default_vertex_cache::reset()
{
	verts_.clear();

	pvs_ = get_weak_handle(pparent_->get_vertex_shader());
	psa_ = &(pparent_->get_geometry_assembler()->get_stream_assembler());
	pvp_ = &(pparent_->get_viewport());
}

template <typename T>
void default_vertex_cache::transform_vertex_impl(const std::vector<T>& indices, atomic<int32_t>& working_index, int32_t index_count)
{
	int32_t local_working_index = (working_index ++).value();

	while (local_working_index < index_count)
	{
		T id = indices[local_working_index];

		if (id < vert_base_){
			custom_assert(false, "");
		}

		size_t pos = id - vert_base_;
	
		if (pos > verts_.size()){
			custom_assert(false, "");
		}

		pvs_->execute(psa_->fetch_vertex(id), verts_[pos]);
		update_wpos(verts_[pos], *pvp_);

		local_working_index = (working_index ++).value();
	}
}

void default_vertex_cache::transform_vertices(const std::vector<uint16_t>& indices)
{
	std::vector<uint16_t> unique_indices = indices;
	std::sort(unique_indices.begin(), unique_indices.end());
	unique_indices.erase(std::unique(unique_indices.begin(), unique_indices.end()), unique_indices.end());
	verts_.resize(unique_indices.size());

	atomic<int32_t> working_index(0);
	// TODO: use a thread pool
	boost::thread_group transform_threads;
	for (size_t i = 0; i < num_cpu_cores(); ++ i)
	{
		transform_threads.create_thread(boost::bind(&default_vertex_cache::transform_vertex_impl<uint16_t>, this, boost::ref(unique_indices), boost::ref(working_index), unique_indices.size()));
	}
	transform_threads.join_all();
}

void default_vertex_cache::transform_vertices(const std::vector<uint32_t>& indices)
{
	std::vector<uint32_t> unique_indices = indices;
	std::sort(unique_indices.begin(), unique_indices.end());
	unique_indices.erase(std::unique(unique_indices.begin(), unique_indices.end()), unique_indices.end());
	verts_.resize(unique_indices.size());

	atomic<int32_t> working_index(0);
	// TODO: use a thread pool
	boost::thread_group transform_threads;
	for (size_t i = 0; i < num_cpu_cores(); ++ i)
	{
		transform_threads.create_thread(boost::bind(&default_vertex_cache::transform_vertex_impl<uint32_t>, this, boost::ref(unique_indices), boost::ref(working_index), unique_indices.size()));
	}
	transform_threads.join_all();
}

vs_output& default_vertex_cache::fetch(cache_entry_index id)
{
	static vs_output null_obj;

	if(id < vert_base_){
		custom_assert(false, "");
		return null_obj;
	}

	size_t pos = id - vert_base_;

	if( pos > verts_.size() ){
		custom_assert(false, "");
		return null_obj;
	}

	return verts_[pos];

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

	if(verts_pool_.is_from(pvert))
	{
		pvert->~vs_output();
		verts_pool_.free(pvert);
	}
}