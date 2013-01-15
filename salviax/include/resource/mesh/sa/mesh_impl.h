#ifndef SALVIAX_RESOURCE_MESH_IMPL_H
#define SALVIAX_RESOURCE_MESH_IMPL_H

#include <salviax/include/resource/mesh/sa/mesh.h>

BEGIN_NS_SALVIAX_RESOURCE();

class mesh_impl : public mesh
{
public:
	salviar::renderer* device_;
	boost::shared_ptr<attached_data> attached_;

	// Buffer informations.
	salviar::h_buffer index_buffer_;
	salviar::h_buffer adjacancies_;
	std::vector<salviar::h_buffer>	vertex_buffers_;
	std::vector<size_t>				strides_;
	std::vector<size_t>				offsets_;
	std::vector<size_t>				slots_;
	
	// Primitive and input descriptions
	size_t primcount_;
	salviar::format index_format_;
	std::vector<salviar::input_element_desc> elem_descs_;
	salviar::h_input_layout cached_layout_;

	// Constructor
	mesh_impl(salviar::renderer* psr);

	// Implements mesh
	virtual size_t get_buffer_count();
	virtual size_t get_face_count();
	virtual salviar::h_buffer get_index_buffer();
	virtual salviar::h_buffer get_vertex_buffer( size_t buffer_index );
	virtual h_attached_data get_attached();
	virtual void gen_adjancency();
	virtual void render();

	// mesh_impl implements
	virtual salviar::h_buffer create_buffer( size_t size );	
	virtual void set_index_buffer( salviar::h_buffer const& );
	virtual void set_index_type(salviar::format index_fmt);
	virtual void add_vertex_buffer( size_t slot, salviar::h_buffer const&, size_t stride, size_t offset );
	virtual void set_primitive_count(size_t primcount);
	virtual void set_input_element_descs(const std::vector<salviar::input_element_desc>& descs);
	virtual void set_attached_data( h_attached_data const& attached );
};

END_NS_SALVIAX_RESOURCE();

#endif