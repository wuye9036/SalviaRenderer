#ifndef SALVIAR_HOST_H
#define SALVIAR_HOST_H

#include <salviar/include/salviar_forward.h>

#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/string/ustring.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

class  stream_assembler;
class  render_parameters;
struct shader_profile;
EFLIB_DECLARE_CLASS_SHARED_PTR(buffer);
EFLIB_DECLARE_CLASS_SHARED_PTR(sampler);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
EFLIB_DECLARE_CLASS_SHARED_PTR(vx_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR(px_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR(host);

class host
{
public:
	virtual void initialize				(stream_assembler* sa) = 0;

	virtual void buffers_changed		() = 0;
	virtual void input_layout_changed	() = 0;

	virtual void update_vertex_shader	(shader_object_ptr const& vso) = 0;
	// virtual void update_pixel_shader	(shader_object_ptr const& pso) = 0;
	// virtual void update_target_params	(render_parameters const& rp, buffer_ptr const& target) = 0;

	virtual bool vx_set_constant		(eflib::fixed_string const&, void const* value) = 0;
	virtual bool vx_set_constant_pointer(eflib::fixed_string const&, void const* pvalue, size_t sz) = 0;
	virtual bool vx_set_sampler			(eflib::fixed_string const&, salviar::sampler_ptr const& samp) = 0;

	virtual vx_shader_unit_ptr	get_vx_shader_unit() const = 0;
	virtual px_shader_unit_ptr	get_px_shader_unit() const = 0;

	virtual ~host() {}
};


END_NS_SALVIAR();

#endif