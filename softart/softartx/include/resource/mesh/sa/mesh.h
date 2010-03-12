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

#ifndef SOFTARTX_MESH_H
#define SOFTARTX_MESH_H

#include "softartx/include/resource/resource_forward.h"
#include "softart/include/decl.h"
#include "softart/include/stream_assembler.h"
#include <boost/smart_ptr.hpp>
#include <vector>

BEGIN_NS_SOFTARTX_RESOURCE()

class base_mesh
{
public:
	virtual size_t get_buffer_count() = 0;
	virtual size_t get_face_count() = 0;

	virtual softart::h_buffer get_buffer(size_t buf_id) = 0;
	virtual softart::h_buffer get_index_buffer() = 0;
	virtual softart::h_buffer get_vertex_buffer() = 0;

	virtual void gen_adjancency() = 0;

	virtual void render(const softart::input_layout_decl& layout) = 0;
	virtual void render() = 0;
};

class mesh : public base_mesh
{
	std::vector<softart::h_buffer> bufs_;
	softart::h_buffer adjacancies_;

	softart::renderer* pdev_;

	size_t idxbufid_;
	size_t vertbufid_;

	size_t primcount_;
	softart::index_type idxtype_;

	std::vector<softart::input_element_decl> default_layout_;

public:
	mesh(softart::renderer* psr);

	/*
	inherited
	*/
	virtual size_t get_buffer_count();
	virtual size_t get_face_count();

	virtual softart::h_buffer get_buffer(size_t buf_id);
	virtual softart::h_buffer get_index_buffer();
	virtual softart::h_buffer get_vertex_buffer();

	virtual void gen_adjancency();

	virtual void render(const softart::input_layout_decl& layout);
	virtual void render();

	/*
	mesh
	*/
	virtual void set_buffer_count(size_t bufcount);
	virtual softart::h_buffer create_buffer(size_t bufid, size_t size);
	
	virtual void set_index_buf_id(size_t bufid);
	virtual void set_vertex_buf_id(size_t bufid);

	virtual void set_primitive_count(size_t primcount);
	virtual void set_index_type(softart::index_type idxtype);

	virtual void set_default_layout(const std::vector<softart::input_element_decl>& layout);
};

DECL_HANDLE(base_mesh, h_mesh);

END_NS_SOFTARTX_RESOURCE()

#endif