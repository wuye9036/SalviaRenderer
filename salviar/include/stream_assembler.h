#ifndef SALVIAR_STREAM_ASSEMBLER_H
#define SALVIAR_STREAM_ASSEMBLER_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/render_stage.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SALVIAR();

class  vs_input;
struct input_element_desc;

EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);
EFLIB_DECLARE_CLASS_WEAK_PTR  (input_layout);

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
	void set_input_layout(input_layout_ptr const&);
	void set_vertex_buffers(
		size_t starts_slot,
		size_t buffers_count, buffer_ptr const* pbufs,
		size_t const* strides, size_t const* offsets
		);

	virtual input_layout_ptr layout() const;

	// Used by Cpp Vertex Shader
	void update_register_map( boost::unordered_map<semantic_value, size_t> const& reg_map );
	void fetch_vertex(vs_input& vertex, size_t vert_index) const;

	// Used by Old Shader Unit
	void const* element_address( input_element_desc const&, size_t vert_index ) const;
	void const* element_address( semantic_value const&, size_t vert_index ) const;

	// Used by New Shader Unit
	virtual std::vector<stream_desc> const& get_stream_descs(std::vector<size_t> const& slots);

	size_t num_vertices() const;

private:
	void const* element_address( size_t buffer_index, size_t member_offset, size_t vert_index ) const;

	/** Find buffer index by slot number.
		@param slot
			Slot number
		@return
			Buffer index found. If slot is invalid, it return -1.
	*/
	int find_buffer( size_t slot ) const;

	// Used by Cpp Vertex Shader
	std::vector<
		boost::tuple<size_t, input_element_desc const*, size_t>
	> reg_and_pelem_and_buffer_index;

	// Used by new shader unit
	std::vector<stream_desc>
							stream_descs_;
	input_layout_weak_ptr	layout_;
	std::vector<size_t>		slots_;
	std::vector<buffer_ptr>	vbufs_;
	std::vector<size_t>		strides_;
	std::vector<size_t>		offsets_;

	// Dirty flags
	bool					layout_dirty_;
	bool					buffer_dirty_;
};

END_NS_SALVIAR()

#endif