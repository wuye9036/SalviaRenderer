#pragma once

#include <salviar/include/salviar_forward.h>

BEGIN_NS_SALVIAR();

struct geom_setup_context
{
	vs_output_op const*	vso_ops;
	vertex_cache*		dvc;
	viewport const*		vp;
};

class geom_setup_engine
{	
public:
	void update(geom_setup_context*);
	
	vs_output** get_geometries();
	
private:
	void clip_geometries();
	void compact_geometries();
	
	void threaded_clip_geometries();
	void threaded_compact_geometries();
	
	boost::shared_array<vs_output_pool>		vso_pools_;
	boost::shared_array<vs_output*>			clipped_verts_;
	boost::shared_array<uint32_t>			package_clipped_verts_count_;
	
	geom_setup_context						ctxt_;
};

END_NS_SALVIAR();