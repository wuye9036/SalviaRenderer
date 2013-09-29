#ifndef SALVIAR_INPUT_LAYOUT_H
#define SALVIAR_INPUT_LAYOUT_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/format.h>
#include <salviar/include/shader.h>

#include <eflib/include/utility/shared_declaration.h>

#include <vector>

BEGIN_NS_SALVIAR();

EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_vertex_shader);

enum input_classifications{
	input_per_vertex

	// TODO:
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

EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);

class input_layout
{
public:
	static input_layout_ptr create( input_element_desc const* pdesc, size_t desc_count, shader_object_ptr const& vs );
	static input_layout_ptr create( input_element_desc const* pdesc, size_t desc_count, cpp_vertex_shader_ptr const& vs );

	typedef std::vector<input_element_desc>::const_iterator iterator;

	iterator desc_begin() const;
	iterator desc_end() const;

	semantic_value get_semantic( iterator it ) const;

	size_t find_slot( semantic_value const& ) const;

	virtual input_element_desc const* find_desc( size_t slot ) const;
	virtual input_element_desc const* find_desc( semantic_value const& ) const;

	void slot_range( size_t& min_slot, size_t& max_slot ) const;

private:
	std::vector<input_element_desc> descs;
};

size_t hash_value(input_element_desc const&);
size_t hash_value(input_layout const&);

END_NS_SALVIAR();

#endif