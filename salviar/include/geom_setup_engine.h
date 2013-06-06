#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/enums.h>

#include <eflib/include/memory/pool.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_array.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

struct vs_output_op;
class  vs_output;
class  vertex_cache;
struct viewport;
struct thread_context;

struct geom_setup_context
{
	vs_output_op const*	vso_ops;
	vertex_cache*		dvc;
	prim_type			prim;
	size_t				prim_size;
	size_t				prim_count;
	bool				(*cull)(float area);
};

/*
	Primitive Verts -> Primitive Packages -> Clipped Packages (Sparse) -> Compacted Clipped Primitive Verts
*/
class geom_setup_engine
{	
public:
	void execute(geom_setup_context const*);

	size_t verts_count() const
	{
		return static_cast<size_t>( clipped_package_compacted_addresses_[clipping_package_count_] );
	}

	vs_output** verts() const
	{
		return compacted_verts_.get();
	}
	
private:
	geom_setup_engine();

	typedef eflib::pool::preserved_pool<vs_output> vs_output_pool;

	void clip_geometries();
	void compact_geometries();
	
	void threaded_clip_geometries(thread_context const* thread_ctx);
	void threaded_compact_geometries(thread_context const* thread_ctx);
	
	boost::shared_array<vs_output_pool>		vso_pools_;
	boost::shared_array<vs_output*>			clipped_verts_;
	boost::shared_array<uint32_t>			clipped_package_verts_count_;
	
	boost::shared_array<vs_output*>			compacted_verts_;

	boost::shared_array<uint32_t>			clipped_package_compacted_addresses_;
	int32_t									clipping_package_count_;

	size_t									thread_count_;
	
	geom_setup_context const*				ctxt_;
};

END_NS_SALVIAR();