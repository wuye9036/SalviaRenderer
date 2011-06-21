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
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
#include <boost/smart_ptr.hpp>
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
#include <vector>

BEGIN_NS_SALVIAX_RESOURCE()

class base_mesh
{
public:
	virtual size_t get_buffer_count() = 0;
	virtual size_t get_face_count() = 0;

	virtual salviar::h_buffer get_buffer(size_t buf_id) = 0;
	virtual salviar::h_buffer get_index_buffer() = 0;
	virtual salviar::h_buffer get_vertex_buffer() = 0;

	virtual void gen_adjancency() = 0;

	virtual void render(const salviar::h_input_layout& layout) = 0;
	virtual void render() = 0;
};

class mesh : public base_mesh
{
	std::vector<salviar::h_buffer> bufs_;
	salviar::h_buffer adjacancies_;

	salviar::renderer* pdev_;

	size_t idxbufid_;
	size_t vertbufid_;

	size_t primcount_;
	salviar::index_type idxtype_;

	std::vector<salviar::input_element_decl> default_layout_;

public:
	mesh(salviar::renderer* psr);

	/*
	inherited
	*/
	virtual size_t get_buffer_count();
	virtual size_t get_face_count();

	virtual salviar::h_buffer get_buffer(size_t buf_id);
	virtual salviar::h_buffer get_index_buffer();
	virtual salviar::h_buffer get_vertex_buffer();

	virtual void gen_adjancency();

	virtual void render(const salviar::h_input_layout& layout);
	virtual void render();

	/*
	mesh
	*/
	virtual void set_buffer_count(size_t bufcount);
	virtual salviar::h_buffer create_buffer(size_t bufid, size_t size);
	
	virtual void set_index_buf_id(size_t bufid);
	virtual void set_vertex_buf_id(size_t bufid);

	virtual void set_primitive_count(size_t primcount);
	virtual void set_index_type(salviar::index_type idxtype);

	virtual void set_default_layout(const std::vector<salviar::input_element_decl>& layout);
};

DECL_HANDLE(base_mesh, h_mesh);

END_NS_SALVIAX_RESOURCE()

#endif