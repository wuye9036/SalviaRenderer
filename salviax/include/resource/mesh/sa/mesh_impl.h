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
	salviar::buffer_ptr index_buffer_;
	salviar::buffer_ptr adjacancies_;
	std::vector<salviar::buffer_ptr>	vertex_buffers_;
	std::vector<size_t>				strides_;
	std::vector<size_t>				offsets_;
	std::vector<size_t>				slots_;
	
	// Primitive and input descriptions
	size_t primcount_;
	salviar::format index_format_;
	std::vector<salviar::input_element_desc> elem_descs_;
	salviar::input_layout_ptr cached_layout_;

	// Constructor
	mesh_impl(salviar::renderer* psr);

	// Implements mesh
	virtual size_t get_buffer_count();
	virtual size_t get_face_count();
	virtual salviar::buffer_ptr get_index_buffer();
	virtual salviar::buffer_ptr get_vertex_buffer( size_t buffer_index );
	virtual attached_data_ptr get_attached();
	virtual void gen_adjancency();
	virtual void render();

	// mesh_impl implements
	virtual salviar::buffer_ptr create_buffer( size_t size );	
	virtual void set_index_buffer( salviar::buffer_ptr const& );
	virtual void set_index_type(salviar::format index_fmt);
	virtual void add_vertex_buffer( size_t slot, salviar::buffer_ptr const&, size_t stride, size_t offset );
	virtual void set_primitive_count(size_t primcount);
	virtual void set_input_element_descs(const std::vector<salviar::input_element_desc>& descs);
	virtual void set_attached_data( attached_data_ptr const& attached );
};

END_NS_SALVIAX_RESOURCE();

#endif