#ifndef SALVIAR_HOST_H
#define SALVIAR_HOST_H

#include <salviar/include/salviar_forward.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

BEGIN_NS_SALVIAR();

class render_parameters;
struct shader_profile;
EFLIB_DECLARE_CLASS_SHARED_PTR(buffer);
EFLIB_DECLARE_CLASS_SHARED_PTR(host);
EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
EFLIB_DECLARE_CLASS_SHARED_PTR(vx_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR(px_shader_unit);

class host
{
public:
	host_ptr create();

	virtual void set_input_layout	(input_layout_ptr const& il) = 0;
	virtual void set_vertex_shader	(shader_object_ptr const& vso) = 0;
	virtual void set_pixel_shader	(shader_object_ptr const& pso) = 0;
	virtual void set_target_params	(render_parameters const& rp, buffer_ptr const& target) = 0;
	
	virtual vx_shader_unit_ptr	get_vx_shader_unit() const = 0;
	virtual px_shader_unit_ptr	get_px_shader_unit() const = 0;

	virtual shader_object_ptr	compile(
		shader_profile const& profile,
		std::string const& code,
		shader_log_ptr& log
		) = 0;

	virtual ~host() {}
};


END_NS_SALVIAR();

#endif