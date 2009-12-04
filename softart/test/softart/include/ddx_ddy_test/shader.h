/********************************************************************
Copyright (C) 2007-2008 Ye Wu

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

created:	2008/06/06
author:		Ye Wu

purpose:	
	提供了测试ddx ddy所需要的shader
Modify Log:
		
*********************************************************************/

#include <softart/include/shader.h>

class vs_shader: public vertex_shader
{
	static efl::mat44 worldViewProjMat_;
public:
	vs_shader(const efl::mat44& worldViewProjMat)
	{
		worldViewProjMat_ = worldViewProjMat;
	}

	void shader_prog(const vs_input& in, vs_output& out)
	{
		const efl::vec4& inpos = in[0];

		transform(out.position, worldViewProjMat_, inpos);
		out.attributes[0].xyz(out.position.xyz());
		out.attributes[0].w = 0.0f;
	}
};

class ps_shader: public pixel_shader
{
	bool shader_prog(const vs_output& in, ps_output& out)
	{
		out.color[0] = in.attributes[0];
		return true;
	}
};

class ddx_ps_shader:public pixel_shader
{
	bool shader_prog(const vs_output& /*in*/, ps_output& out)
	{
		out.color[0] = ddx(0);
		return true;
	}
};

class bs_shader: public blend_shader
{
	bool shader_prog(backbuffer_pixel_out& inout, const backbuffer_pixel_in& in)
	{
		if(in.depth() < inout.depth())
		{
			inout.color(0, in.color(0));
			inout.depth() = in.depth();
		}
		return true;
	}
};