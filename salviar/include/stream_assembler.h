#ifndef SALVIAR_STREAM_ASSEMBLER_H
#define SALVIAR_STREAM_ASSEMBLER_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>
#include <salviar/include/render_stage.h>

BEGIN_NS_SALVIAR();

class stream_assembler
{
public:
	void set_input_layout( input_layout const* );
	void set_vertex_buffers( size_t starts_slot, size_t buffers_count, h_buffer* pbufs, size_t const* strides, size_t const* offsets );

	input_layout const* layout() const;
	std::vector<h_buffer> const& streams() const;

	void fetch_vertex(vs_input& vertex, size_t idx);
	size_t num_vertices() const;

private:
	input_layout* layout_;

	std::vector<size_t>		slots_;

	std::vector<h_buffer>	vbufs_;
	
	std::vector<size_t>		strides_;
	std::vector<size_t>		offsets_;
};

END_NS_SALVIAR()

#endif