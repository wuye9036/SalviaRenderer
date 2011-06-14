#include <salviar/include/shader_unit.h>

#include <salviar/include/shader_code.h>
#include <salviar/include/renderer.h>
#include <salviar/include/buffer.h>

#include <sasl/include/semantic/abi_info.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using namespace sasl::semantic;
using std::vector;

BEGIN_NS_SALVIAR();

void vertex_shader_unit::initialize( shader_code const* code ){
	this->code = code;
	this->stream_data.resize( code->abii()->storage_size(stream_in), 0 );
	this->buffer_data.resize( code->abii()->storage_size(buffer_in), 0 );
}

void vertex_shader_unit::bind_streams( vector<input_element_decl> const& layout, vector<h_buffer> const& streams ){
	this->layout = layout;
	this->streams = streams;
}

void vertex_shader_unit::update( size_t ivert )
{
	abi_info const* abii = code->abii();
	BOOST_FOREACH( input_element_decl const& elem_decl, layout ){
		if( elem_decl.usage == input_register_usage_position ){
			void* psrc = streams[elem_decl.stream_idx]->raw_data( elem_decl.stride*ivert + elem_decl.offset );
			storage_info* pos_si = abii->input_storage( SV_Position );
			void* pdest = &(stream_data[pos_si->offset]);
			*static_cast<intptr_t*>(pdest) = reinterpret_cast<intptr_t>(psrc);
		} else {
			// TODO Ingore others
		}
	}
}

void vertex_shader_unit::execute( vs_output& out )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void vertex_shader_unit::set_variable( std::string const&, void* data )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

vertex_shader_unit::vertex_shader_unit()
: code(NULL)
{
}

vertex_shader_unit::~vertex_shader_unit()
{
}
END_NS_SALVIAR();