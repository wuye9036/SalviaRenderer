/*
Copyright (C) 2004-2005 Minmin Gong

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "shader.h"
#include "softart/include/renderer.h"
#include "softart/include/renderer_impl.h"
#include "softart/include/framebuffer.h"
#include "softartx/include/resource/mesh_io.h"
#include "softartx/include/resource/tex_io.h"

#include <boost/smart_ptr.hpp>

#include <tchar.h>
#include <iostream>


using namespace efl;
using namespace boost;

int _tmain(int /*argc*/, _TCHAR* /*argv[]*/)
{
	renderer_parameters rp;
	rp.backbuffer_format = pixel_format_color_rgba32f;
	rp.backbuffer_height = 64;
	rp.backbuffer_width = 64;

	h_renderer hrend = create_software_renderer(&rp, h_device());

	h_mesh box = create_box(hrend.get());

	hrend->clear_color(0, rect<int>(0, 0, 64, 64), color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
	hrend->clear_depth(rect<int>(0, 0, 64, 64), 1.0f);

	vec4 camera(1.5f, 1.5f, 1.5f, 1.0f);
	
	mat44 world(mat44::identity()), view, proj, wvp;

	mat_lookat(view, camera, vec4::gen_coord(0.0f, 0.0f, 0.0f), vec4::gen_vector(0.0f, 1.0f, 0.0f));
	mat_perspective_fov(proj, float(PI) / 2.0f, 1.0f, 0.1f, 100.0f);
	mat_mul(wvp, mat_mul(wvp, proj, view), world);

	h_vertex_shader hvs = h_vertex_shader(new vs_shader(wvp));
	h_pixel_shader hps = h_pixel_shader(new ps_shader);
	h_blend_shader hbs = h_blend_shader(new bs_shader);

	hrend->set_vertex_shader(hvs);
	hrend->set_pixel_shader(hps);
	hrend->set_blend_shader(hbs);

	hrend->set_cull_mode(cull_none);
	box->render();

	save_surface_to_raw_text(*(static_pointer_cast<renderer_impl>(hrend)->get_framebuffer()->get_render_target(render_target_color, 0)), _EFLIB_T("c:/normal_rendering.raw"), 'R');

	hrend->clear_color(0, rect<int>(0, 0, 64, 64), color_rgba32f(0.2f, 0.2f, 0.5f, 1.0f));
	hrend->clear_depth(rect<int>(0, 0, 64, 64), 1.0f);

	hps = h_pixel_shader(new ddx_ps_shader);
	hrend->set_pixel_shader(hps);

	hrend->set_cull_mode(cull_none);
	box->render();

	save_surface_to_raw_text(*(static_pointer_cast<renderer_impl>(hrend)->get_framebuffer()->get_render_target(render_target_color, 0)), _EFLIB_T("c:/ddx_rendering.raw"), 'R');
	return 0;
}