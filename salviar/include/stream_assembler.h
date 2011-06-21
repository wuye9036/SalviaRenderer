#ifndef SALVIAR_STREAM_ASSEMBLER_H
#define SALVIAR_STREAM_ASSEMBLER_H

#include "decl.h"
#include "render_stage.h"
#include "stream.h"
#include <salviar/include/salviar_forward.h>
BEGIN_NS_SALVIAR()


class stream_assembler// : public render_stage
{
	input_layout_decl layout_;
	std::vector<h_buffer> streams_;

public:
	void set_input_layout(const input_layout_decl& layout);
	void set_stream(stream_index stridx, h_buffer hbuf);

	std::vector<input_element_decl> const& layout() const;
	std::vector<h_buffer> const& streams() const;

	void fetch_vertex(vs_input& vertex, size_t idx);
	size_t num_vertices() const;
};

END_NS_SALVIAR()

#endif