#pragma once

#ifndef SALVIAR_RASTERIZER_H
#define SALVIAR_RASTERIZER_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>
#include <salviar/include/enums.h>
#include <salviar/include/shader.h>
#include <salviar/include/framebuffer.h>

#include <eflib/include/memory/atomic.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

struct scanline_info;
class pixel_shader_unit;

struct rasterizer_desc {
	fill_mode fm;
	cull_mode cm;
	bool front_ccw;
	int32_t depth_bias;
	float depth_bias_clamp;
	float slope_scaled_depth_bias;
	bool depth_clip_enable;
	bool scissor_enable;
	bool multisample_enable;
	bool anti_aliased_line_enable;

	rasterizer_desc()
		: fm(fill_solid), cm(cull_back),
			front_ccw(false),
			depth_bias(0), depth_bias_clamp(0), slope_scaled_depth_bias(0),
			depth_clip_enable(true), scissor_enable(false),
			multisample_enable(true), anti_aliased_line_enable(false)
	{
	}
};

class rasterizer_state {
	rasterizer_desc desc_;

	typedef bool (*cm_func_type)(float area);
	typedef void (*clipping_func_type)(
		uint32_t& num_clipped_prims, uint32_t& num_out_clipped_verts,
		vs_output* clipped_verts, uint32_t* clipped_indices,
		uint32_t base_vertex,
		const h_clipper& clipper, const viewport& vp, const vs_output** pv, cm_func_type cm_fn,
		const vs_output_op& vs_output_ops
		);
	typedef void (*triangle_rast_func_type)(
		uint32_t& /*Primitive Size*/, 
		boost::function< void (
			rasterizer*,
			const uint32_t*, const vs_output*,
			const std::vector<uint32_t>&, const viewport&,
			const h_pixel_shader&, boost::shared_ptr<pixel_shader_unit> const&)
		>& /*Rasterizer Function*/ );

	cm_func_type cm_func_;
	clipping_func_type clipping_func_;
	triangle_rast_func_type triangle_rast_func_;

public:
	rasterizer_state(const rasterizer_desc& desc);
	const rasterizer_desc& get_desc() const;

	bool cull(float area) const;
	void clipping(
		uint32_t& num_clipped_prims, uint32_t& num_out_clipped_verts,
		vs_output* clipped_verts, uint32_t* clipped_indices,
		uint32_t base_vertex,
		const h_clipper& clipper, const viewport& vp, const vs_output** pv,
		const vs_output_op& vs_output_ops
		) const;

	void triangle_rast_func(uint32_t& prim_size,
		boost::function<
			void (
			rasterizer*,
			const uint32_t*, const vs_output*,
			const std::vector<uint32_t>&, const viewport&,
			const h_pixel_shader&, boost::shared_ptr<pixel_shader_unit> const&)
		>& rasterize_func
		) const;
};

class rasterizer : public render_stage
{
	const static int MAX_NUM_MULTI_SAMPLES = 4;

	h_rasterizer_state	state_;
	uint32_t			num_vs_output_attributes_;

	h_framebuffer		frame_buffer_;
	h_blend_shader		blend_shader_;

	std::vector<eflib::vec3> edge_factors_;
	eflib::vec2 samples_pattern_[MAX_NUM_MULTI_SAMPLES];

	void geometry_setup_func(
		uint32_t* num_clipped_prims, vs_output* clipped_verts, uint32_t* cliped_indices,
		int32_t prim_count, primitive_topology primtopo,
		boost::atomic<int32_t>& working_package, int32_t package_size
		);

	void dispatch_primitive_func(
		std::vector<std::vector<uint32_t> >& tiles, uint32_t const* clipped_indices, vs_output const* clipped_verts_full,
		int32_t prim_count, uint32_t stride,
		boost::atomic<int32_t>& working_package, int32_t package_size
		);
	
	void rasterize_primitive_func(
		std::vector<std::vector<std::vector<uint32_t> > >& thread_tiles, int num_tiles_x,
		const uint32_t* clipped_indices, const vs_output* clipped_verts_full,
		const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu,
		boost::atomic<int32_t>& working_package, int32_t package_size
		);
	
	void compact_clipped_verts_func(
		uint32_t* clipped_indices, const uint32_t* cliiped_indices_full,
		const uint32_t* addresses, const uint32_t* num_clipped_prims,
		int32_t prim_count,
		boost::atomic<int32_t>& working_package, int32_t package_size
		);

	boost::function<
		void ( rasterizer*, const uint32_t* /*Clipped Indexes*/, const vs_output* /*Clipped Vertexes*/, 
			const std::vector<uint32_t>& /*Sorted Primitives*/, const viewport& /*Tile VP*/, 
			const h_pixel_shader& /*Pixel Shader*/, boost::shared_ptr<pixel_shader_unit> const& /*Pixel Shader Unit*/)
	> rasterize_func_;

	void draw_whole_tile(
		int left, int top, int right, int bottom,
		size_t num_samples, const vs_output& v0,
		const vs_output& ddx, const vs_output& ddy, const vs_output_op* vs_output_ops,
		const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu, const h_blend_shader& hbs,
		const float* aa_z_offset );
	void draw_pixels(
		int left0, int top0, int left, int top,
		const eflib::vec4* edge_factors, size_t num_samples, bool has_centroid,
		const vs_output& v0, const vs_output& ddx, const vs_output& ddy, const vs_output_op* vs_output_ops,
		const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu, const h_blend_shader& hbs, 
		const float* aa_z_offset);
	void subdivide_tile(int left, int top, const eflib::rect<uint32_t>& cur_region, const eflib::vec4* edge_factors,
		uint32_t* test_regions, uint32_t& test_region_size, float x_min, float x_max, float y_min, float y_max,
		const float* rej_to_acc, const float* evalue, const float* step_x, const float* step_y);

	void draw_full_package(
		vs_output* pixels,
		uint32_t top, uint32_t left, size_t num_samples,
		h_blend_shader const& bs, h_pixel_shader const& pps, boost::shared_ptr<pixel_shader_unit> const& psu,
		float const* aa_z_offset );
	void draw_package(
		vs_output* pixels,
		uint32_t top, uint32_t left, size_t num_samples,
		h_blend_shader const& bs, h_pixel_shader const& pps, boost::shared_ptr<pixel_shader_unit> const& psu,
		uint32_t const* masks, float const* aa_z_offset );

public:
	//inherited
	void initialize(renderer_impl* pparent);

	//constructor
	rasterizer();
	~rasterizer();

	//state_seter
	void set_state(const h_rasterizer_state& state);
	const h_rasterizer_state& get_state() const;

	//drawer
	void rasterize_line(
		uint32_t prim_id, const vs_output& v0, const vs_output& v1, const viewport& vp,
		const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu);
	void rasterize_triangle(
		uint32_t prim_id, uint32_t full, const vs_output& v0, const vs_output& v1, const vs_output& v2,
		const viewport& vp,
		const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu);

	void rasterize_line_func(
		const uint32_t* clipped_indices, const vs_output* clipped_verts_full,
		const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp,
		const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu );
	void rasterize_triangle_func(
		const uint32_t* clipped_indices, const vs_output* clipped_verts_full, 
		const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp,
		const h_pixel_shader& pps, boost::shared_ptr<pixel_shader_unit> const& psu );

	void draw(size_t prim_count);
};

//DECL_HANDLE(rasterizer, h_rasterizer);

END_NS_SALVIAR()

#endif