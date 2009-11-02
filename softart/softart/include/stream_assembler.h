#ifndef SOFTART_STREAM_ASSEMBLER_H
#define SOFTART_STREAM_ASSEMBLER_H

#include "render_stage.h"
#include "stream.h"

class vs_input_impl;

class stream_assembler// : public render_stage
{
	input_layout_decl layout_;
	std::vector<h_buffer> streams_;

public:
	void set_input_layout(const input_layout_decl& layout);
	void set_stream(stream_index stridx, h_buffer hbuf);

	vs_input_impl fetch_vertex(size_t idx);
};

#endif