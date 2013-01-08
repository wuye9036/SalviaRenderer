#include <salviar/include/renderer.h>

#include <salviar/include/renderer_impl.h>
#include <salviar/include/async_renderer.h>

BEGIN_NS_SALVIAR();

h_renderer create_software_renderer(renderer_parameters const* pparam, h_device const& hdev)
{
	// return create_renderer_impl(pparam, hdev);
	return create_async_renderer(pparam, hdev);
}

shader_object_ptr	compile(std::string const& /*code*/, shader_profile const& /*profile*/, shader_log_ptr& /*logs*/)
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shader_object_ptr();
}

shader_object_ptr	compile(std::string const& code, shader_profile const& profile)
{
	shader_log_ptr log;
	return compile(code, profile, log);
}

shader_object_ptr	compile(std::string const& code, languages lang)
{
	shader_profile prof;
	prof.language = lang;
	return compile(code, prof);
}

END_NS_SALVIAR();