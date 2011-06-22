/********************************************************************
Copyright (C) 2007-2010 Ye Wu

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

created:	2008/06/08
author:		Ye Wu

purpose:	提供了网格模型的接口声明与平台无关定义文件。

Modify Log:
		
*********************************************************************/

#ifndef SALVIAX_MESH_H
#define SALVIAX_MESH_H

#include "salviax/include/resource/resource_forward.h"
#include "salviar/include/decl.h"
#include "salviar/include/stream_assembler.h"

#include <eflib/include/platform/boost_begin.h>
#include <boost/smart_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SALVIAX_RESOURCE();

class base_mesh
{
public:
	virtual size_t get_buffer_count() = 0;
	virtual size_t get_face_count() = 0;

	virtual salviar::h_buffer get_buffer(size_t buffer_index) = 0;
	virtual salviar::h_buffer get_index_buffer() = 0;
	virtual salviar::h_buffer get_vertex_buffer() = 0;

	virtual void gen_adjancency() = 0;

	virtual void render( salviar::h_input_layout const& layout, h_shader_code const& shader_code ) = 0;
	virtual void render() = 0;
};

class mesh : public base_mesh
{
	std::vector<salviar::h_buffer>	vertex_buffers_;
	std::vector<size_t>				strides_;
	std::vector<size_t>				offsets_;
	std::vector<size_t>				slots_;

	salviar::h_buffer index_buffer_;

	salviar::h_buffer adjacancies_;

	salviar::renderer* device_;

	size_t vertex_buffer_index_;

	size_t primcount_;
	salviar::index_type index_fmt_;

	std::vector<salviar::input_element_desc> elem_descs_;

public:
	mesh(salviar::renderer* psr);

	// Implements base_mesh
	virtual size_t get_buffer_count();
	virtual size_t get_face_count();

	virtual salviar::h_buffer get_buffer(size_t buf_id);
	virtual salviar::h_buffer get_index_buffer();
	virtual salviar::h_buffer get_vertex_buffer();

	virtual void gen_adjancency();

	virtual void render( salviar::h_input_layout const& layout, h_shader_code const& shader_code );
	virtual void render();

	//@{
	// ! Mesh specific functions.
	virtual salviar::h_buffer create_buffer( size_t size );
	
	virtual void set_index_buffer( salviar::h_buffer const& );

	virtual void add_vertex_buffer( size_t slot, salviar::h_buffer const&, size_t stride, size_t offset );

	virtual void set_primitive_count(size_t primcount);
	virtual void set_index_type(salviar::index_type idxtype);

	virtual void set_default_layout(const std::vector<salviar::input_element_desc>& layout);
	//}@
};

DECL_HANDLE(base_mesh, h_mesh);

END_NS_SALVIAX_RESOURCE();

#endif