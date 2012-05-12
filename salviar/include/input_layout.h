#ifndef SALVIAR_INPUT_LAYOUT_H
#define SALVIAR_INPUT_LAYOUT_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/format.h>
#include <salviar/include/shader.h>

#include <vector>

BEGIN_NS_SALVIAR();

enum input_classifications{
	input_per_vertex

	// TODO
	// input_per_instance
};

struct input_element_desc
{
	std::string				semantic_name;
	uint32_t				semantic_index;
	format					data_format;
	uint32_t				input_slot;
	uint32_t				aligned_byte_offset;
	input_classifications	slot_class;

	uint32_t				instance_data_step_rate;
	
	input_element_desc(
		const char* semantic_name,
		uint32_t semantic_index,
		format data_format,
		uint32_t input_slot,
		uint32_t aligned_byte_offset,
		input_classifications slot_class,
		uint32_t instance_data_step_rate
		)
		: semantic_name( semantic_name )
		, semantic_index( semantic_index )
		, data_format( data_format )
		, input_slot( input_slot )
		, aligned_byte_offset( aligned_byte_offset )
		, slot_class( slot_class )
		, instance_data_step_rate( instance_data_step_rate )
	{}

	input_element_desc()
		: semantic_index(0)
		, data_format(format_unknown)
		, input_slot(0), aligned_byte_offset(0xFFFFFFFF)
		, slot_class(input_per_vertex), instance_data_step_rate(0)
	{}
};

class input_layout{
public:
	static h_input_layout create( input_element_desc const* pdesc, size_t desc_count, h_shader_code const& vs );
	static h_input_layout create( input_element_desc const* pdesc, size_t desc_count, h_vertex_shader const& vs );

	typedef std::vector<input_element_desc>::const_iterator iterator;

	iterator desc_begin() const;
	iterator desc_end() const;

	semantic_value get_semantic( iterator it ) const;

	/** Get a slot number that is belonged to a input elememt description has the same semantic value as parameter.
		@param sv
			The semantic value used to look up.
		@return 
			If the semantic is exists, tt return the found slot number, otherwise return the maximum value of size_t.
	*/
	size_t find_slot( semantic_value const& ) const;

	input_element_desc const* find_desc( size_t slot ) const;
	input_element_desc const* find_desc( semantic_value const& ) const;

	void slot_range( size_t& min_slot, size_t& max_slot ) const;

private:
	std::vector<input_element_desc> descs;
};

END_NS_SALVIAR();

#endif