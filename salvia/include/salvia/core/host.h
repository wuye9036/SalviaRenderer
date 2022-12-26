#ifndef SALVIAR_HOST_H
#define SALVIAR_HOST_H

#include <salviar/include/salviar_forward.h>

#include <eflib/utility/shared_declaration.h>
#include <eflib/string/ustring.h>

#include <memory>

namespace salviar{

class  stream_assembler;
struct renderer_parameters;
struct shader_profile;
struct render_stages;
struct render_state;
EFLIB_DECLARE_CLASS_SHARED_PTR(buffer);
EFLIB_DECLARE_CLASS_SHARED_PTR(sampler);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
EFLIB_DECLARE_CLASS_SHARED_PTR(vx_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR(px_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR(host);

class host
{
public:
	virtual void initialize				(render_stages const* stages) = 0;
	virtual void update					(render_state const* state) = 0;

	// virtual void update_target_params	(renderer_parameters const& rp, buffer_ptr const& target) = 0;

	virtual vx_shader_unit_ptr	get_vx_shader_unit() const = 0;
	virtual px_shader_unit_ptr	get_px_shader_unit() const = 0;
	virtual size_t				vs_output_attr_count() const = 0;

	virtual ~host() {}
};


}

#endif