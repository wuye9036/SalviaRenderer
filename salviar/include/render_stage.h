#ifndef SALVIAR_RENDER_STAGE_H
#define SALVIAR_RENDER_STAGE_H

#include "decl.h"
#include "enums.h"

#include <boost/any.hpp>
#include <salviar/include/salviar_forward.h>
BEGIN_NS_SALVIAR()


//声明 RenderState Updated 事件的默认处理函数
#define DECL_RS_UPDATED(rsname) virtual result on_##rsname##_updated(){return result::ok;}

//声明 RenderState Updated的处理函数
#define PROC_RS_UPDATED(rsname) virtual result on_##rsname##_updated();

//实现RenderState Updated事件处理
#define IMPL_RS_UPDATED(class_name, rsname) result class_name::on_##rsname##_updated()

//调用RenderStage
#define DISPATCH_RS_UPDATED(hobj, rsname) hobj->on_##rsname##_updated();

class render_stage
{
protected:
	renderer_impl* pparent_;
	render_stage():pparent_(NULL){}

public:
	virtual void initialize(renderer_impl* pparent) = 0;

	DECL_RS_UPDATED(input_layout);
	DECL_RS_UPDATED(stream);
	DECL_RS_UPDATED(index_buffer);
	DECL_RS_UPDATED(primitive_topology);
	DECL_RS_UPDATED(vertex_shader);
	DECL_RS_UPDATED(viewport);
	DECL_RS_UPDATED(cull_mode);
	DECL_RS_UPDATED(fill_mode);
	DECL_RS_UPDATED(pixel_shader);
	DECL_RS_UPDATED(blend_shader);
	DECL_RS_UPDATED(framebuffer_size);
	DECL_RS_UPDATED(render_target);
	DECL_RS_UPDATED(rt_avaliable);

	virtual ~render_stage(){}
};

END_NS_SALVIAR()

#endif