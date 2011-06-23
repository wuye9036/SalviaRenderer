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

#include <salviax/include/resource/mesh/sa/mesh.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/resource_manager.h>

#include <eflib/include/diagnostics/assert.h>

using namespace std;
using namespace eflib;
using namespace salviar;

BEGIN_NS_SALVIAX_RESOURCE();

mesh::mesh(salviar::renderer* psr)
{
	EFLIB_ASSERT(psr, "");

	device_ = psr;
}

/*
inherited
*/
size_t mesh::get_buffer_count()
{
	return vertex_buffers_.size();
}

size_t mesh::get_face_count()
{
	return primcount_;
}

salviar::h_buffer mesh::get_index_buffer()
{
	return index_buffer_;
}

salviar::h_buffer mesh::get_vertex_buffer( size_t buffer_index )
{
	if( buffer_index < vertex_buffers_.size() ){
		return vertex_buffers_[buffer_index];
	} else {
		return h_buffer();
	}
}

void mesh::gen_adjancency(){
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void mesh::render(){
	EFLIB_ASSERT(device_, "");
	if(!device_) return;
	
	if ( !device_->get_vertex_shader_code() ){ return; }

	cached_layout_ = device_->create_input_layout(
		&( elem_descs_[0] ), elem_descs_.size(),
		device_->get_vertex_shader_code()
		);
	if( !cached_layout_ ){ return; }
	for(size_t i_buffer = 0; i_buffer < vertex_buffers_.size(); ++i_buffer){
		device_->set_vertex_buffers(
			slots_[i_buffer], 1, &vertex_buffers_[i_buffer],
			&(strides_[i_buffer]), &(offsets_[i_buffer])
			);
	}

	device_->set_index_buffer( get_index_buffer(), index_fmt_ );
	device_->set_input_layout( cached_layout_ );
	device_->set_primitive_topology( primitive_triangle_list );

	device_->draw_index(0, primcount_, 0);
}

/*
mesh
*/
salviar::h_buffer mesh::create_buffer( size_t size ){
	return device_->create_buffer(size);
}

void mesh::set_primitive_count(size_t primcount)
{
	primcount_ = primcount;
}

void mesh::set_index_buffer( salviar::h_buffer const& v ){
	index_buffer_ = v;
}

void mesh::set_index_type( index_type fmt )
{
	switch(fmt){
		case index_int16:
		case index_int32:
			index_fmt_ = fmt;
			break;
		default:
			EFLIB_ASSERT(false, "");
	}
}

void mesh::add_vertex_buffer( size_t slot, salviar::h_buffer const& buf, size_t stride, size_t offset ){
	vertex_buffers_.push_back( buf );
	strides_.push_back( stride );
	offsets_.push_back( offset );
	slots_.push_back( slot );
}

void mesh::set_input_element_descs(const vector<input_element_desc>& descs){
	elem_descs_ = descs;
}

END_NS_SALVIAX_RESOURCE();