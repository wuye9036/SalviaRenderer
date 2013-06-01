#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>
#include <salviar/include/enums.h>
#include <salviar/include/shader.h>
#include <salviar/include/framebuffer.h>
#include <salviar/include/raster_state.h>

#include <eflib/include/memory/atomic.h>
#include <eflib/include/memory/pool.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

typedef eflib::pool::preserved_pool<vs_output> vs_output_pool;

struct scanline_info;
class  pixel_shader_unit;
class  vs_output;
struct vs_output_op;
struct clip_context;

struct geometry_setup_context
{
	/* Outputs */
	vs_output**				clipped_vertices;
	uint32_t*				num_clipped_vertices;

	/* Inputs */
	vs_output_pool*			vso_pool;
	int32_t					num_primitive;
	primitive_topology		topo;
	boost::atomic<int32_t>*	working_package;
	int32_t					package_size;
};

struct dispatch_primitive_context
{
	typedef std::vector<std::vector<uint32_t>> tile_list;

	/* Outputs */
	tile_list*				tiles;
	boost::atomic<int32_t>*	working_package;

	/* Inputs */
	uint32_t const*			clipped_indices;
	vs_output const**		clipped_verts_full;
	int32_t					prim_count;
	uint32_t				stride;
	int32_t					package_size;
};

struct drawing_context
{
	int					vp_left, vp_top;
	int					rgn_left, rgn_top, rgn_right, rgn_bottom;
	eflib::vec4 const*	edge_factors;
	size_t				num_samples;
	bool				has_centroid;
	vs_output const*	v0;
	vs_output const*	ddx;
	vs_output const*	ddy;
	vs_output const*	pixels;
	vs_output_op const*	vs_output_ops;
	pixel_shader*		cpp_ps;
	pixel_shader_unit*	ps_unit;
	blend_shader*		cpp_bs;
	uint32_t const*		masks;
	float const*		aa_z_offset;
};

class rasterizer : public render_stage
{
private:
	const static int MAX_NUM_MULTI_SAMPLES = 4;

	std::vector<clipper>	clippers_;
	raster_state_ptr		state_;
	uint32_t				num_vs_output_attributes_;

	framebuffer_ptr			frame_buffer_;
	blend_shader_ptr			blend_shader_;

	std::vector<eflib::vec3> edge_factors_;
	eflib::vec2 samples_pattern_[MAX_NUM_MULTI_SAMPLES];

	prim_type				prim_;
	uint32_t				prim_size_;

	void geometry_setup_func(geometry_setup_context const* ctxt);
	void dispatch_primitive_func(
		std::vector<std::vector<uint32_t> >& tiles,
		vs_output** clipped_verts,
		int32_t prim_count, uint32_t stride,
		boost::atomic<int32_t>& working_package, int32_t package_size
		);
	
	void rasterize_primitive_func(
		std::vector<std::vector<std::vector<uint32_t> > >& thread_tiles, int num_tiles_x,
		vs_output** clipped_verts_full,
		const pixel_shader_ptr& pps, boost::shared_ptr<pixel_shader_unit> const& psu,
		boost::atomic<int32_t>& working_package, int32_t package_size
		);
	
	void compact_clipped_verts_func(
		vs_output** compacted, vs_output** sparse,
		uint32_t const* addresses, uint32_t const* num_clipped_verts,
		int32_t workset, boost::atomic<int32_t>& working_package, int32_t package_size
	);

	boost::function<
		void ( rasterizer*, vs_output** /*Clipped Vertexes*/, 
			const std::vector<uint32_t>& /*Sorted Primitives*/, const viewport& /*Tile VP*/, 
			const pixel_shader_ptr& /*Pixel Shader*/, boost::shared_ptr<pixel_shader_unit> const& /*Pixel Shader Unit*/)
	> rasterize_func_;

	void draw_whole_tile(
		int left, int top, int right, int bottom,
		size_t num_samples, const vs_output& v0,
		const vs_output& ddx, const vs_output& ddy, const vs_output_op* vs_output_ops,
		const pixel_shader_ptr& pps, boost::shared_ptr<pixel_shader_unit> const& psu, const blend_shader_ptr& hbs,
		const float* aa_z_offset );
	void draw_pixels(
		int left0, int top0, int left, int top,
		const eflib::vec4* edge_factors, size_t num_samples, bool has_centroid,
		const vs_output& v0, const vs_output& ddx, const vs_output& ddy, const vs_output_op* vs_output_ops,
		const pixel_shader_ptr& pps, boost::shared_ptr<pixel_shader_unit> const& psu, const blend_shader_ptr& hbs, 
		const float* aa_z_offset);
	void subdivide_tile(int left, int top, const eflib::rect<uint32_t>& cur_region, const eflib::vec4* edge_factors,
		uint32_t* test_regions, uint32_t& test_region_size, float x_min, float x_max, float y_min, float y_max,
		const float* rej_to_acc, const float* evalue, const float* step_x, const float* step_y);

	void draw_full_package(
		vs_output* pixels,
		uint32_t top, uint32_t left, size_t num_samples,
		blend_shader_ptr const& bs, pixel_shader_ptr const& pps, boost::shared_ptr<pixel_shader_unit> const& psu,
		float const* aa_z_offset );
	void draw_package(
		vs_output* pixels,
		uint32_t top, uint32_t left, size_t num_samples,
		blend_shader_ptr const& bs, pixel_shader_ptr const& pps, boost::shared_ptr<pixel_shader_unit> const& psu,
		uint32_t const* masks, float const* aa_z_offset );

	void viewport_and_project_transform(vs_output** vertexes, size_t num_verts);

public:
	//inherited
	void initialize(renderer_impl* pparent);

	//constructor
	rasterizer();
	~rasterizer();

	//state_seter
	void set_state(const raster_state_ptr& state);
	const raster_state_ptr& get_state() const;

	//drawer
	void rasterize_line(
		uint32_t prim_id, const vs_output& v0, const vs_output& v1, const viewport& vp,
		const pixel_shader_ptr& pps, boost::shared_ptr<pixel_shader_unit> const& psu);
	void rasterize_triangle(
		uint32_t prim_id, uint32_t full, const vs_output& v0, const vs_output& v1, const vs_output& v2,
		const viewport& vp,
		const pixel_shader_ptr& pps, boost::shared_ptr<pixel_shader_unit> const& psu);

	void rasterize_line_func(
		vs_output** clipped_verts_full,
		const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp,
		const pixel_shader_ptr& pps, boost::shared_ptr<pixel_shader_unit> const& psu );
	void rasterize_triangle_func(
		vs_output** clipped_verts_full, 
		const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp,
		const pixel_shader_ptr& pps, boost::shared_ptr<pixel_shader_unit> const& psu );

	void draw(size_t prim_count);

	void update_prim_info();
};

END_NS_SALVIAR();