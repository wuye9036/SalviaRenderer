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
#include <salviax/include/resource/mesh/sa/mesh_impl.h>
#include <salviar/include/input_layout.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/resource_manager.h>

#include <eflib/include/math/quaternion.h>
#include <eflib/include/math/math.h>

using namespace std;
using namespace eflib;
using namespace salviar;

BEGIN_NS_SALVIAX_RESOURCE();

//0, 0, 0 - 1, 1, 1
mesh_ptr create_box(salviar::renderer* psr)
{
	mesh_impl* pmesh = new mesh_impl(psr);

	size_t const geometry_slot	= 0;
	size_t const normal_slot	= 1;
	size_t const uv_slot		= 2;

	salviar::buffer_ptr indices	= pmesh->create_buffer( sizeof(uint16_t)*36 );

	salviar::buffer_ptr verts		= pmesh->create_buffer( sizeof(vec4)*24 );
	salviar::buffer_ptr normals	= pmesh->create_buffer( sizeof(vec4)*24 );
	salviar::buffer_ptr uvs		= pmesh->create_buffer( sizeof(vec4)*24 );

	// Generate data
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

	pmesh->add_vertex_buffer( geometry_slot, verts, sizeof(vec4), 0 );
	pmesh->add_vertex_buffer( normal_slot, normals, sizeof(vec4), 0 );
	pmesh->add_vertex_buffer( uv_slot, uvs, sizeof(vec4), 0 );

	vector<input_element_desc> descs;

	descs.push_back( input_element_desc( "POSITION", 0, format_r32g32b32a32_float, geometry_slot, 0, input_per_vertex, 0 ) );
	descs.push_back( input_element_desc( "NORMAL",   0, format_r32g32b32a32_float, normal_slot,   0, input_per_vertex, 0 ) );
	descs.push_back( input_element_desc( "TEXCOORD", 0, format_r32g32b32a32_float, uv_slot,       0, input_per_vertex, 0 ) );

	pmesh->set_input_element_descs( descs );

	pmesh->set_primitive_count(12);

	return mesh_ptr(pmesh);
}

mesh_ptr create_planar(
					 salviar::renderer* rend,
					 const eflib::vec3& start_pos,
					 const eflib::vec3& x_dir,	 const eflib::vec3& y_dir,
					 size_t repeat_x, size_t repeat_y,
					 bool positive_normal
					 )
{
	mesh_impl* pmesh = new mesh_impl(rend);

	size_t nverts = (repeat_x + 1) * (repeat_y + 1);

	size_t const geometry_slot	= 0;
	size_t const normal_slot	= 1;
	size_t const uv_slot		= 2;

	salviar::buffer_ptr indices	= pmesh->create_buffer( repeat_x * repeat_y * 6 * sizeof(uint16_t) );

	salviar::buffer_ptr verts	= pmesh->create_buffer( nverts * sizeof(vec4) );
	salviar::buffer_ptr normals	= pmesh->create_buffer( nverts * sizeof(vec4) );
	salviar::buffer_ptr uvs		= pmesh->create_buffer( nverts * sizeof(vec4) );

	//Generate data
 	vec4 normal(normalize3(cross_prod3(x_dir, y_dir)), 0.0f);
	if(!positive_normal) normal = -normal;
	vec4 line_spos(start_pos, 1.0f);
	vec4 x(x_dir, 0.0f);
	vec4 y(y_dir, 0.0f);
	size_t offset_v = 0;

	for(size_t i = 0; i < repeat_x + 1; ++i) {
		vec4 pos = line_spos;
		for(size_t j = 0; j < repeat_y + 1; ++j) {
			vec4 uv(
				static_cast<float>(i)/repeat_x,
				static_cast<float>(j)/repeat_y,
				0.0f, 0.0f
				);

			verts->transfer(offset_v, &pos, sizeof(vec4), sizeof(vec4), sizeof(vec4), 1);
			normals->transfer(offset_v, &normal, sizeof(vec4), sizeof(vec4), sizeof(vec4), 1);
			uvs->transfer(offset_v, &uv, sizeof(vec4), sizeof(vec4), sizeof(vec4), 1);
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

	pmesh->add_vertex_buffer( geometry_slot, verts, sizeof(vec4), 0 );
	pmesh->add_vertex_buffer( normal_slot, normals, sizeof(vec4), 0 );
	pmesh->add_vertex_buffer( uv_slot, uvs, sizeof(vec4), 0);

	vector<input_element_desc> descs;
	descs.push_back( input_element_desc( "POSITION", 0, format_r32g32b32a32_float, geometry_slot, 0, input_per_vertex, 0 ) );
	descs.push_back( input_element_desc( "NORMAL",   0, format_r32g32b32a32_float, normal_slot,   0, input_per_vertex, 0 ) );
	descs.push_back( input_element_desc( "TEXCOORD", 0, format_r32g32b32a32_float, uv_slot,       0, input_per_vertex, 0 ) );

	pmesh->set_input_element_descs( descs );

	pmesh->set_primitive_count(repeat_x * repeat_y * 2);

	return mesh_ptr(pmesh);
}

mesh_ptr create_planar(
	salviar::renderer* rend,
	eflib::vec3 const& norm,
	eflib::vec3 const& start_pos,
	eflib::vec3 const& major_dir,
	eflib::vec2 const& length,
	size_t			   repeat_x,
	size_t			   repeat_y,
	bool			   positive_normal
	)
{
	eflib::vec3 n = norm;
	if(norm.length_sqr() < eflib::epsilon)
	{
		return mesh_ptr();
	}

	if(major_dir.length_sqr() < eflib::epsilon)
	{
		return mesh_ptr();
	}

	vec3 binorm = cross_prod3(major_dir, norm);
	if(binorm.length_sqr() < eflib::epsilon)
	{
		return mesh_ptr();
	}

	vec3 tangent = cross_prod3(norm, binorm);
	if(tangent.length_sqr() < eflib::epsilon)
	{
		return mesh_ptr();
	}

	n = normalize3(norm);
	binorm = normalize3(binorm) * length.x();
	tangent = normalize3(tangent) * length.y();

	return create_planar(rend, start_pos, binorm, tangent, repeat_x, repeat_y, positive_normal);
}

mesh_ptr create_cone(
	salviar::renderer* psr,
	eflib::vec3 const& bottom_center,
	float radius, eflib::vec3 const& up_dir, int circle_segments)
{
	if( circle_segments < 3 )
	{
		circle_segments = 3;
	}

	// Pick an guide vector for generating bottom plane
	vec3 guide_vector;
	if( eflib::equal(up_dir.x(), 0.0f) )
	{
		guide_vector = vec3(1.0f, 0.0f, 0.0f);
	}
	else if( eflib::equal(up_dir.y(), 0.0f) )
	{
		guide_vector = vec3(0.0f, 1.0f, 0.0f);
	}
	else
	{
		guide_vector = vec3(0.0f, 0.0f, 1.0f);
	}

	// Compute local X axis of bottom plane
	vec3 local_x_axis = cross_prod3(up_dir, guide_vector);
	local_x_axis.normalize();
	local_x_axis *= radius;

	float segment_angle = static_cast<float>(eflib::TWO_PI/circle_segments);
	quaternion seg_rotation = quaternion::from_axis_angle(up_dir, segment_angle);

	// Compute all vertexes
	vec3 top_vertex = bottom_center + up_dir;

	vector<vec3> circle_vertices;
	vec3 rotated_radial_amount = local_x_axis;
	for(int i_seg = 0; i_seg < circle_segments; ++i_seg)
	{
		circle_vertices.push_back(rotated_radial_amount + bottom_center);
		eflib::transform( rotated_radial_amount, seg_rotation, rotated_radial_amount);
	}
	circle_vertices.push_back( circle_vertices[0] );

	// Compute face normals
	vector<vec3> face_normals;
	for(int i_seg = 0; i_seg < circle_segments; ++i_seg)
	{
		vec3 edge0 = circle_vertices[i_seg] - top_vertex;
		vec3 edge1 = circle_vertices[i_seg+1] - circle_vertices[i_seg];
		vec3 face_normal = cross_prod3(edge0, edge1);
		face_normals.push_back(face_normal);
	}

	// Compute bottom circle vert normals
	vector<vec3> circle_normals;
	circle_normals.push_back( (face_normals.back() + face_normals[0]) * 0.5f );
	for(int i_seg = 1; i_seg < circle_segments; ++i_seg)
	{
		circle_normals.push_back( (face_normals[i_seg-1] + face_normals[i_seg]) * 0.5f );
	}

	// Compute UVs
	vec2 top_uv = vec2(0.5f, 0.5f);
	vector<vec2> circle_uvs;
	for(int i_seg = 0; i_seg < circle_segments; ++i_seg)
	{
		float u = sin(segment_angle*i_seg) * 0.5f + 0.5f;
		float v = cos(segment_angle*i_seg) * 0.5f + 0.5f;
		circle_uvs.push_back( vec2(u, v) );
	}

	// Fill buffers
	mesh_impl* pmesh = new mesh_impl(psr);

	size_t const geometry_slot = 0;
	size_t const normal_slot = 1;
	size_t const uv_slot = 2;

	buffer_ptr indices = pmesh->create_buffer( sizeof(uint16_t)*circle_segments*3 );
	buffer_ptr verts   = pmesh->create_buffer( sizeof(vec4)*circle_segments*2 );
	buffer_ptr norms   = pmesh->create_buffer( sizeof(vec4)*circle_segments*2 );
	buffer_ptr uvs     = pmesh->create_buffer( sizeof(vec4)*circle_segments*2 );

	// Fill vertex buffer
	vec4* vert_cursor = reinterpret_cast<vec4*>( verts->raw_data(0) );
	for(int i_seg = 0; i_seg < circle_segments; ++i_seg)
	{
		*vert_cursor = vec4(top_vertex, 1.0f);
		++vert_cursor;
		*vert_cursor = vec4(circle_vertices[i_seg], 1.0f);
		++vert_cursor;
	}

	// Fill normal buffer
	vec4* norm_cursor = reinterpret_cast<vec4*>( norms->raw_data(0) );
	for(int i_seg = 0; i_seg < circle_segments; ++i_seg)
	{
		*norm_cursor = vec4(face_normals[i_seg], 1.0f);
		++norm_cursor;
		*norm_cursor = vec4(circle_normals[i_seg], 1.0f);
		++norm_cursor;
	}

	// Fill UV buffer
	vec4* uv_cursor = reinterpret_cast<vec4*>( uvs->raw_data(0) );
	for(int i_seg = 0; i_seg < circle_segments; ++i_seg)
	{
		*uv_cursor = vec4(top_uv.x(), top_uv.y(), 0.0f, 0.0f);
		++uv_cursor;
		*uv_cursor = vec4(circle_uvs[i_seg].x(), circle_uvs[i_seg].y(), 0.0f, 0.0f);
		++uv_cursor;
	}

	// Fill index buffer
	uint16_t* index_cursor = reinterpret_cast<uint16_t*>( indices->raw_data(0) );
	for(int i_seg = 0; i_seg < circle_segments; ++i_seg)
	{
		*index_cursor = static_cast<uint16_t>(i_seg*2); // Top
		++index_cursor;
		*index_cursor = static_cast<uint16_t>((i_seg*2+3) % (circle_segments*2));
		++index_cursor;
		*index_cursor = static_cast<uint16_t>(i_seg*2+1);
		++index_cursor;
	}

	// Set input layout
	pmesh->set_index_type(format_r16_uint);
	pmesh->set_index_buffer( indices );

	pmesh->add_vertex_buffer( geometry_slot, verts, sizeof(vec4), 0 );
	pmesh->add_vertex_buffer( normal_slot, norms, sizeof(vec4), 0 );
	pmesh->add_vertex_buffer( uv_slot, uvs, sizeof(vec4), 0 );

	vector<input_element_desc> descs;

	descs.push_back( input_element_desc( "POSITION", 0, format_r32g32b32a32_float, geometry_slot, 0, input_per_vertex, 0 ) );
	descs.push_back( input_element_desc( "NORMAL",   0, format_r32g32b32a32_float, normal_slot,   0, input_per_vertex, 0 ) );
	descs.push_back( input_element_desc( "TEXCOORD", 0, format_r32g32b32a32_float, uv_slot,       0, input_per_vertex, 0 ) );

	pmesh->set_input_element_descs( descs );

	pmesh->set_primitive_count(circle_segments);

	return mesh_ptr(pmesh);
}

END_NS_SALVIAX_RESOURCE();
