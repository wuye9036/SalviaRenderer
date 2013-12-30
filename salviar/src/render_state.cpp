#include <salviar/include/render_state.h>
#include <salviar/include/shader.h>
#include <salviar/include/shader_cbuffer.h>

BEGIN_NS_SALVIAR();

void copy_using_state(render_state* dest, render_state const* src)
{
    switch(src->cmd)
    {
    case command_id::draw:
    case command_id::draw_index:
        *dest = *src;
        if(src->cpp_vs)
        {
            dest->cpp_vs = src->cpp_vs->clone<cpp_vertex_shader>();
        }
        if(src->cpp_ps)
        {
            dest->cpp_ps = src->cpp_ps->clone<cpp_pixel_shader>();
        }
        if(src->cpp_bs)
        {
            dest->cpp_bs = src->cpp_bs->clone<cpp_blend_shader>();
        }
        break;
    case command_id::clear_color:
    case command_id::clear_depth_stencil:
        dest->cmd                = src->cmd;
        dest->clear_color_target = src->clear_color_target;
        dest->clear_ds_target    = src->clear_ds_target   ;
               
		dest->clear_f			 = src->clear_f			  ;
	    dest->clear_z            = src->clear_z           ;
	    dest->clear_stencil      = src->clear_stencil     ;
        dest->clear_color        = src->clear_color       ;
        break;
    case command_id::async_begin:
    case command_id::async_end:
        dest->cmd                = src->cmd;
        dest->current_async      = src->current_async;
        break;
    }
}

END_NS_SALVIAR();