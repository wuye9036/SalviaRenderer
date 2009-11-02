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

#include "softartx/include/resource/mesh_io.h"

#include "softart/include/renderer_impl.h"
#include "softart/include/resource_manager.h"

using namespace std;
using namespace efl;

//0, 0, 0 - 1, 1, 1
h_mesh create_box(renderer* psr)
{
	mesh* pmesh = new mesh(psr);
	
	const int vertbufid = 0;
	const int normbufid = 1;
	const int idxbufid = 2;

	pmesh->set_buffer_count(3);

	h_buffer verts = pmesh->create_buffer(vertbufid, sizeof(vec4)*24);
	h_buffer indices = pmesh->create_buffer(idxbufid, sizeof(uint16_t)*36);
	h_buffer normals = pmesh->create_buffer(normbufid, sizeof(vec4)*24);

	vec4* pverts = (vec4*)(verts->raw_data(0));
	uint16_t* pidxs = (uint16_t*)(indices->raw_data(0));
	vec4* pnorms = (vec4*)(normals->raw_data(0));

	//+x
	pverts[0] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	pverts[1] = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	pverts[2] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pverts[3] = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	pnorms[0] = pnorms[1] = pnorms[2] = pnorms[3] = vec4(1.0f, 0.0f, 0.0f, 0.0f);

	//-x
	pverts[4] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	pverts[5] = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	pverts[6] = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	pverts[7] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pnorms[4] = pnorms[5] = pnorms[6] = pnorms[7] = vec4(-1.0f, 0.0f, 0.0f, 0.0f);

	//+y
	pverts[8] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pverts[9] = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	pverts[10] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pverts[11] = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	pnorms[8] = pnorms[9] = pnorms[10] = pnorms[11] = vec4(0.0f, 1.0f, 0.0f, 0.0f);

	//-y
	pverts[12] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	pverts[13] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	pverts[14] = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	pverts[15] = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	pnorms[12] = pnorms[13] = pnorms[14] = pnorms[15] = vec4(0.0f, -1.0f, 0.0f, 0.0f);

	//+z
	pverts[16] = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	pverts[17] = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	pverts[18] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pverts[19] = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	pnorms[16] = pnorms[17] = pnorms[18] = pnorms[19] = vec4(0.0f, 0.0f, 1.0f, 0.0f);

	//-z
	pverts[20] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	pverts[21] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pverts[22] = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	pverts[23] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	pnorms[20] = pnorms[21] = pnorms[22] = pnorms[23] = vec4(0.0f, 0.0f, -1.0f, 0.0f);

	pidxs[ 0] = 0;	pidxs[ 1] = 1;	pidxs[ 2] = 2;
	pidxs[ 3] = 2;	pidxs[ 4] = 3;	pidxs[ 5] = 0;
	pidxs[ 6] = 4;	pidxs[ 7] = 5;	pidxs[ 8] = 6;
	pidxs[ 9] = 6;	pidxs[10] = 7;	pidxs[11] = 4;
	pidxs[12] = 8;	pidxs[13] = 9;	pidxs[14] = 10;
	pidxs[15] = 10;	pidxs[16] = 11;	pidxs[17] = 8;	
	pidxs[18] = 12;	pidxs[19] = 13;	pidxs[20] = 14;
	pidxs[21] = 14;pidxs[22] = 15;pidxs[23] = 12;
	pidxs[24] = 16;pidxs[25] = 17;pidxs[26] = 18;
	pidxs[27] = 18;pidxs[28] = 19;pidxs[29] = 16;
	pidxs[30] = 20;pidxs[31] = 21;pidxs[32] = 22;
	pidxs[33] = 22;pidxs[34] = 23;pidxs[35] = 20;

	input_layout_decl layout;
	layout.push_back(input_element_decl(stream_0, 0, sizeof(vec4), input_float4, input_register_usage_position, input_reg_0));
	layout.push_back(input_element_decl(stream_1, 0, sizeof(vec4), input_float4, input_register_usage_attribute, input_reg_1));

	pmesh->set_index_type(index_int16);
	pmesh->set_primitive_count(12);
	pmesh->set_index_buf_id(idxbufid);
	pmesh->set_default_layout(layout);

	return h_mesh(pmesh);
}

h_mesh create_planar(
					 renderer* psr,
					 const efl::vec3& start_pos,
					 const efl::vec3& x_dir,	 const efl::vec3& y_dir,
					 size_t repeat_x, size_t repeat_y,
					 bool positive_normal
					 )
{
	mesh* pmesh = new mesh(psr);

	size_t nverts = (repeat_x + 1) * (repeat_y + 1);

	const size_t vertbufid = 0;
	const size_t norbufid = 1;
	const size_t idxbufid = 2;

	pmesh->set_buffer_count(3);
	h_buffer verts = pmesh->create_buffer(vertbufid, nverts * sizeof(vec4));
	h_buffer nors = pmesh->create_buffer(norbufid, nverts * sizeof(vec4));
	h_buffer idxs = pmesh->create_buffer(idxbufid, repeat_x * repeat_y * 6 * sizeof(uint16_t));

	//构造数据
	vec4 normal(cross_prod3(x_dir, y_dir), 0.0f);
	if(!positive_normal) normal = -normal;
	vec4 line_spos(start_pos, 1.0f);
	vec4 x(x_dir, 0.0f);
	vec4 y(y_dir, 0.0f);
	size_t offset_v = 0;

	for(size_t i = 0; i < repeat_x + 1; ++i)
	{
		vec4 pos = line_spos;
		for(size_t j = 0; j < repeat_y + 1; ++j)
		{
			verts->transfer(offset_v, &pos, sizeof(vec4), sizeof(vec4), sizeof(vec4), 1);
			nors->transfer(offset_v, &normal, sizeof(vec4), sizeof(vec4), sizeof(vec4), 1);
			pos += y;
			offset_v += sizeof(vec4);
		}
		line_spos += x;
	}

	size_t offset_i = 0;
	uint16_t quad[6];
	for(size_t i = 0; i < repeat_x; ++i)
	{
		for(size_t j = 0; j < repeat_y; ++j)
		{
			quad[0] = uint16_t(i * (repeat_y + 1) + j);
			quad[1] = uint16_t(quad[0] + repeat_y + 1);
			quad[2] = uint16_t(quad[0] + 1);

			quad[3] = uint16_t(quad[1]);
			quad[4] = uint16_t(quad[1] + 1);
			quad[5] = uint16_t(quad[0] + 1);

			idxs->transfer(offset_i, &quad[0], sizeof(quad), sizeof(quad), sizeof(quad), 1);
			offset_i += sizeof(quad);
		}
	}

	input_layout_decl layout;
	layout.push_back(input_element_decl(stream_0, 0, sizeof(vec4), input_float4, input_register_usage_position, input_reg_0));
	layout.push_back(input_element_decl(stream_1, 0, sizeof(vec4), input_float4, input_register_usage_attribute, input_reg_1));

	pmesh->set_index_type(index_int16);
	pmesh->set_primitive_count(repeat_x * repeat_y * 2);
	pmesh->set_index_buf_id(idxbufid);
	pmesh->set_default_layout(layout);

	return h_mesh(pmesh);
}