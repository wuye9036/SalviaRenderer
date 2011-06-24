/*
Copyright (C) 2007-2010 Ye Wu, Minmin Gong

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

#include <salviax/include/resource/mesh/sa/mesh_io.h>
#include <salviar/include/input_layout.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/resource_manager.h>

using namespace std;
using namespace eflib;
using namespace salviar;

BEGIN_NS_SALVIAX_RESOURCE();

//0, 0, 0 - 1, 1, 1
h_mesh create_box(salviar::renderer* psr)
{
	mesh* pmesh = new mesh(psr);

	size_t const geometry_slot	= 0;
	size_t const normal_slot	= 1;
	size_t const uv_slot		= 2;

	salviar::h_buffer indices	= pmesh->create_buffer( sizeof(uint16_t)*36 );

	salviar::h_buffer verts		= pmesh->create_buffer( sizeof(vec4)*24 );
	salviar::h_buffer normals	= pmesh->create_buffer( sizeof(vec4)*24 );
	salviar::h_buffer uvs		= pmesh->create_buffer( sizeof(vec4)*24 );

	// Generate datas
	uint16_t* pidxs = reinterpret_cast<uint16_t*>(indices->raw_data(0));

	vec4* pverts = reinterpret_cast<vec4*>(verts->raw_data(0));
	vec4* pnorms = reinterpret_cast<vec4*>(normals->raw_data(0));
	vec4* puvs = reinterpret_cast<vec4*>(uvs->raw_data(0));

	//+x
	pverts[0] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	pverts[1] = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	pverts[2] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pverts[3] = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	pnorms[0] = pnorms[1] = pnorms[2] = pnorms[3] = vec4(1.0f, 0.0f, 0.0f, 0.0f);
	puvs[0] = vec4(0.0f , 0.0f , 0.0f , 0.0f);
	puvs[1] = vec4(1.0f , 0.0f , 0.0f , 0.0f);
	puvs[2] = vec4(1.0f , 1.0f , 0.0f , 0.0f);
	puvs[3] = vec4(0.0f , 1.0f , 0.0f , 0.0f);

	//-x
	pverts[4] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	pverts[5] = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	pverts[6] = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	pverts[7] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pnorms[4] = pnorms[5] = pnorms[6] = pnorms[7] = vec4(-1.0f, 0.0f, 0.0f, 0.0f);
	puvs[4] = vec4(0.0f , 0.0f , 0.0f , 0.0f);
	puvs[5] = vec4(1.0f , 0.0f , 0.0f , 0.0f);
	puvs[6] = vec4(1.0f , 1.0f , 0.0f , 0.0f);
	puvs[7] = vec4(0.0f , 1.0f , 0.0f , 0.0f);

	//+y
	pverts[8] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pverts[9] = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	pverts[10] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pverts[11] = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	pnorms[8] = pnorms[9] = pnorms[10] = pnorms[11] = vec4(0.0f, 1.0f, 0.0f, 0.0f);
	puvs[8] = vec4(0.0f , 0.0f , 0.0f , 0.0f);
	puvs[9] = vec4(1.0f , 0.0f , 0.0f , 0.0f);
	puvs[10] = vec4(1.0f , 1.0f , 0.0f , 0.0f);
	puvs[11] = vec4(0.0f , 1.0f , 0.0f , 0.0f);

	//-y
	pverts[12] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	pverts[13] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	pverts[14] = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	pverts[15] = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	pnorms[12] = pnorms[13] = pnorms[14] = pnorms[15] = vec4(0.0f, -1.0f, 0.0f, 0.0f);
	puvs[12] = vec4(0.0f , 0.0f , 0.0f , 0.0f);
	puvs[13] = vec4(1.0f , 0.0f , 0.0f , 0.0f);
	puvs[14] = vec4(1.0f , 1.0f , 0.0f , 0.0f);
	puvs[15] = vec4(0.0f , 1.0f , 0.0f , 0.0f);

	//+z
	pverts[16] = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	pverts[17] = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	pverts[18] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pverts[19] = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	pnorms[16] = pnorms[17] = pnorms[18] = pnorms[19] = vec4(0.0f, 0.0f, 1.0f, 0.0f);
	puvs[16] = vec4(0.0f , 0.0f , 0.0f , 0.0f);
	puvs[17] = vec4(1.0f , 0.0f , 0.0f , 0.0f);
	puvs[18] = vec4(1.0f , 1.0f , 0.0f , 0.0f);
	puvs[19] = vec4(0.0f , 1.0f , 0.0f , 0.0f);

	//-z
	pverts[20] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	pverts[21] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pverts[22] = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	pverts[23] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	pnorms[20] = pnorms[21] = pnorms[22] = pnorms[23] = vec4(0.0f, 0.0f, -1.0f, 0.0f);
	puvs[20] = vec4(0.0f , 0.0f , 0.0f , 0.0f);
	puvs[21] = vec4(1.0f , 0.0f , 0.0f , 0.0f);
	puvs[22] = vec4(1.0f , 1.0f , 0.0f , 0.0f);
	puvs[23] = vec4(0.0f , 1.0f , 0.0f , 0.0f);

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

	// Set generated data to mesh
	pmesh->set_index_type(format_r16_uint);
	pmesh->set_index_buffer( indices );

	pmesh->add_vertex_buffer( geometry_slot, verts, 24, 0 );
	pmesh->add_vertex_buffer( normal_slot, normals, 24, 0 );
	pmesh->add_vertex_buffer( uv_slot, uvs, 24, 0 );

	vector<input_element_desc> descs;

	descs.push_back( input_element_desc( "POSITION", 0, format_r32g32b32a32_float, geometry_slot, 0, input_per_vertex, 0 ) );
	descs.push_back( input_element_desc( "NORMAL",   0, format_r32g32b32a32_float, normal_slot,   0, input_per_vertex, 0 ) );
	descs.push_back( input_element_desc( "TEXCOORD", 0, format_r32g32b32a32_float, uv_slot,       0, input_per_vertex, 0 ) );

	pmesh->set_input_element_descs( descs );

	pmesh->set_primitive_count(12);

	return h_mesh(pmesh);
}

h_mesh create_planar(
					 salviar::renderer* psr,
					 const eflib::vec3& start_pos,
					 const eflib::vec3& x_dir,	 const eflib::vec3& y_dir,
					 size_t repeat_x, size_t repeat_y,
					 bool positive_normal
					 )
{
	mesh* pmesh = new mesh(psr);

	size_t nverts = (repeat_x + 1) * (repeat_y + 1);

	size_t const geometry_slot	= 0;
	size_t const normal_slot	= 1;

	salviar::h_buffer indices	= pmesh->create_buffer( repeat_x * repeat_y * 6 * sizeof(uint16_t) );

	salviar::h_buffer verts		= pmesh->create_buffer( nverts * sizeof(vec4) );
	salviar::h_buffer normals	= pmesh->create_buffer( nverts * sizeof(vec4) );

	//Generate datas
	vec4 normal(cross_prod3(x_dir, y_dir), 0.0f);
	if(!positive_normal) normal = -normal;
	vec4 line_spos(start_pos, 1.0f);
	vec4 x(x_dir, 0.0f);
	vec4 y(y_dir, 0.0f);
	size_t offset_v = 0;

	for(size_t i = 0; i < repeat_x + 1; ++i) {
		vec4 pos = line_spos;
		for(size_t j = 0; j < repeat_y + 1; ++j) {
			verts->transfer(offset_v, &pos, sizeof(vec4), sizeof(vec4), sizeof(vec4), 1);
			normals->transfer(offset_v, &normal, sizeof(vec4), sizeof(vec4), sizeof(vec4), 1);
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
			quad[1] = uint16_t(quad[0] + 1);
			quad[2] = uint16_t(quad[0] + repeat_y + 1 + 1);

			quad[3] = uint16_t(quad[2]);
			quad[4] = uint16_t(quad[2] - 1);
			quad[5] = uint16_t(quad[0]);

			indices->transfer(offset_i, &quad[0], sizeof(quad), sizeof(quad), sizeof(quad), 1);
			offset_i += sizeof(quad);
		}
	}

	// Set generated data to mesh
	pmesh->set_index_type(format_r16_uint);
	pmesh->set_index_buffer( indices );

	pmesh->add_vertex_buffer( geometry_slot, verts, 24, 0 );
	pmesh->add_vertex_buffer( normal_slot, normals, 24, 0 );

	vector<input_element_desc> descs;
	descs.push_back( input_element_desc( "POSITION", 0, format_r32g32b32a32_float, geometry_slot, 0, input_per_vertex, 0 ) );
	descs.push_back( input_element_desc( "NORMAL",   0, format_r32g32b32a32_float, normal_slot,   0, input_per_vertex, 0 ) );

	pmesh->set_input_element_descs( descs );

	pmesh->set_primitive_count(repeat_x * repeat_y * 2);

	return h_mesh(pmesh);
}

END_NS_SALVIAX_RESOURCE();