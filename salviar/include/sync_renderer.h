#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/renderer_impl.h>

#include <eflib/utility/shared_declaration.h>

namespace salviar{

struct vs_input_op;
struct vs_output_op;

EFLIB_DECLARE_CLASS_SHARED_PTR(host);
EFLIB_DECLARE_CLASS_SHARED_PTR(sync_renderer);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
EFLIB_DECLARE_CLASS_SHARED_PTR(pixel_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR(stream_assembler);

EFLIB_DECLARE_STRUCT_SHARED_PTR(render_state);

class sync_renderer : public renderer_impl
{
public:
	virtual result flush();
    virtual result commit_state_and_command();
	sync_renderer();
};

renderer_ptr create_sync_renderer();

}