#pragma once

#include <salviar/include/salviar_forward.h>

#include <eflib/include/utility/shared_declaration.h>

#include <vector>
#include <tuple>
#include <unordered_map>


namespace salviar{

class  vs_input;
struct input_element_desc;
struct render_state;
struct stream_buffer_desc;

EFLIB_DECLARE_CLASS_SHARED_PTR(buffer);
EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);

class semantic_value;

struct stream_desc
{
	void*	buffer;
	size_t	offset;
	size_t	stride;
};

class stream_assembler
{
public:
	void update(render_state const* state);

	// Used by Cpp Vertex Shader
	void update_register_map( std::unordered_map<semantic_value, size_t> const& reg_map );
	void fetch_vertex(vs_input& vertex, size_t vert_index) const;

	// Used by Old Shader Unit
	void const* element_address( input_element_desc const&, size_t vert_index ) const;
	void const* element_address( semantic_value const&, size_t vert_index ) const;

	// Used by New Shader Unit
	virtual std::vector<stream_desc> const& get_stream_descs(std::vector<size_t> const& slots);

private:
	// Used by Cpp Vertex Shader
	struct reg_ied_extra_t
	{
		size_t reg_id;
		input_element_desc const* desc;
		float default_wcomp;
	};
	std::vector<reg_ied_extra_t> reg_ied_extra_;

	// Used by new shader unit
	std::vector<stream_desc>	stream_descs_;
	input_layout*				layout_;
	stream_buffer_desc const*	stream_buffer_descs_;	
};

}
