#ifndef SALVIAR_STREAM_ASSEMBLER_H
#define SALVIAR_STREAM_ASSEMBLER_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>
#include <salviar/include/render_stage.h>

#include <vector>

BEGIN_NS_SALVIAR();

class semantic_value;

class stream_assembler
{
public:
	void set_input_layout( input_layout const* );

	void set_vertex_buffers(
		size_t starts_slot,
		size_t buffers_count, h_buffer const* pbufs,
		size_t const* strides, size_t const* offsets
		);

	input_layout const* layout() const;

	void fetch_vertex(vs_input& vertex, size_t vert_index) const;

	void const* element_address( input_element_desc const&, size_t vert_index ) const;
	void const* element_address( semantic_value const&, size_t vert_index ) const;

	size_t num_vertices() const;

private:
	int buffer_index( size_t slot ) const;

	input_layout const* layout_;

	std::vector<size_t>		slots_;

	std::vector<h_buffer>	vbufs_;
	
	std::vector<size_t>		strides_;
	std::vector<size_t>		offsets_;
};

END_NS_SALVIAR()

#endif