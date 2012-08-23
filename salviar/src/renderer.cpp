#include <salviar/include/renderer.h>

#include <salviar/include/renderer_impl.h>
#include <salviar/include/async_renderer.h>

BEGIN_NS_SALVIAR();

h_renderer create_software_renderer(renderer_parameters const* pparam, h_device const& hdev)
{
	// return create_renderer_impl(pparam, hdev);
	return create_async_renderer(pparam, hdev);
}

END_NS_SALVIAR();