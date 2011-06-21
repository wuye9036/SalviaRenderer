#ifndef SALVIAR_SHADER_ABI_H
#define SALVIAR_SHADER_ABI_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/shader.h>

BEGIN_NS_SALVIAR();

enum language_value_types
{
	lvt_none,

	lvt_sint8,
	lvt_sint16,
	lvt_sint32,
	lvt_sint64
};

enum storage_classifications{
	sc_none = 0,

	sc_stream_in,
	sc_stream_out,
	sc_buffer_in,
	sc_buffer_out,
	
	storage_classfications_count
};

struct storage_info{
	storage_info();
	int						index;
	int						offset;
	int						size;
	storage_classifications	storage;
	shader_value_format		format;
};

END_NS_SALVIAR();

#endif