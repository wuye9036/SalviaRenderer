#ifndef SALVIAX_MESH_H
#define SALVIAX_MESH_H

#include <salviax/include/resource/resource_forward.h>
#include <salviar/include/decl.h>
#include <salviar/include/stream_assembler.h>
#include <salviar/include/input_layout.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SALVIAX_RESOURCE();

class attached_data{
public:
	virtual ~attached_data() = 0{}
};

DECL_HANDLE(attached_data, h_attached_data);

class base_mesh
{
public:
	virtual size_t get_buffer_count() = 0;
	virtual size_t get_face_count() = 0;

	virtual salviar::h_buffer get_index_buffer() = 0;
	virtual salviar::h_buffer get_vertex_buffer( size_t buffer_index ) = 0;
	
	virtual h_attached_data get_attached() = 0;

	virtual void gen_adjancency() = 0;

	virtual void render() = 0;
};

class mesh : public base_mesh
{
private:
	salviar::renderer* device_;

	/** Members for mesh data
	@{*/
	salviar::h_buffer index_buffer_;
	salviar::h_buffer adjacancies_;

	std::vector<salviar::h_buffer>	vertex_buffers_;
	std::vector<size_t>				strides_;
	std::vector<size_t>				offsets_;
	std::vector<size_t>				slots_;

	boost::shared_ptr<attached_data> attached_;
	/**@}*/

	/** Members for rendering
	@{*/
	size_t primcount_;
	salviar::format index_fmt_;
	std::vector<salviar::input_element_desc> elem_descs_;
	salviar::h_input_layout cached_layout_;
	/**@}*/
public:
	mesh(salviar::renderer* psr);

	/** Implements base_mesh
	@{*/ 
	virtual size_t get_buffer_count();
	virtual size_t get_face_count();

	virtual salviar::h_buffer get_index_buffer();
	virtual salviar::h_buffer get_vertex_buffer( size_t buffer_index );

	virtual h_attached_data get_attached();
	
	virtual void gen_adjancency();

	virtual void render();
	/**@}*/

	/** Mesh specific functions.
	@{*/
	virtual salviar::h_buffer create_buffer( size_t size );
	
	virtual void set_index_buffer( salviar::h_buffer const& );
	virtual void set_index_type(salviar::format index_fmt);

	virtual void add_vertex_buffer( size_t slot, salviar::h_buffer const&, size_t stride, size_t offset );

	virtual void set_primitive_count(size_t primcount);
	
	virtual void set_input_element_descs(const std::vector<salviar::input_element_desc>& descs);

	virtual void set_attached_data( h_attached_data const& attached );
	/**@}*/
};

DECL_HANDLE(base_mesh, h_mesh);

END_NS_SALVIAX_RESOURCE();

#endif

/********************************************************************
Copyright (C) 2007-2012 Ye Wu

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
*********************************************************************/
