#include "../include/vertex_cache.h"

#include "../include/shader.h"
#include "../include/shaderregs_op.h"
#include "../include/renderer_impl.h"
#include "../include/stream_assembler.h"
#include "../include/geometry_assembler.h"

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
	btransformed_.clear();

	pvs_ = get_weak_handle(pparent_->get_vertex_shader());
	psa_ = &(pparent_->get_geometry_assembler()->sa_);
	pvp_ = &(pparent_->get_viewport());
}

void default_vertex_cache::resize(size_t s)
{
	verts_.resize(s);
	btransformed_.insert(btransformed_.end(), s - btransformed_.size(), false);
}

vs_output& default_vertex_cache::fetch(cache_entry_index id)
{
	static vs_output null_obj;

	if(id < vert_base_)
	{
		custom_assert(false, "");
		return null_obj;
	}

	size_t pos = id - vert_base_;

	if( pos > verts_.size() ){
		resize(id+1);
	}

	if(! btransformed_[pos]){
		pvs_->execute(psa_->fetch_vertex(id), verts_[pos]);
		update_wpos(verts_[pos], *pvp_);
		btransformed_[pos] = true;
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