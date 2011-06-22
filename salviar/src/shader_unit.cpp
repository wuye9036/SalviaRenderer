#include <salviar/include/shader_unit.h>

#include <salviar/include/shader_code.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/renderer.h>
#include <salviar/include/buffer.h>
#include <salviar/include/stream_assembler.h>

#include <sasl/include/semantic/abi_info.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/math/math.h>
#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using namespace sasl::semantic;
using namespace eflib;
using std::vector;

BEGIN_NS_SALVIAR();

void vertex_shader_unit::initialize( shader_code const* code ){
	this->code = code;
	this->stream_data.resize( code->abii()->total_size( sc_stream_in), 0 );
	this->buffer_data.resize( code->abii()->total_size(sc_buffer_in), 0 );
	this->stream_odata.resize( code->abii()->total_size(sc_stream_out), 0 );
	this->buffer_odata.resize( code->abii()->total_size(sc_buffer_out), 0 );
}

void vertex_shader_unit::bind_streams( stream_assembler const* sa ){
	this->sa = sa;
}

void vertex_shader_unit::update( size_t ivert )
{
	abi_info const* abii = code->abii();
	vector<storage_info*> infos = abii->storage_infos( sc_stream_in );

	BOOST_FOREACH( storage_info* info, infos ){
		void const* psrc = sa->element_address( info->sv, ivert );
		assert( psrc );
		void* pdest = &(stream_data[info->offset]);

		*static_cast<void const* *>(pdest) = psrc;
	}
}

void vertex_shader_unit::execute( vs_output& out )
{
	void (*p)(void*, void*, void*, void*)
		= static_cast<void (*)(void*, void*, void*, void*)>( code->function_pointer() );

	void* psi = stream_data.empty() ? NULL : &(stream_data[0]);
	void* pbi = buffer_data.empty() ? NULL : &(buffer_data[0]);
	void* pso = stream_odata.empty() ? NULL : &(stream_odata[0]);
	void* pbo = buffer_odata.empty() ? NULL : &(buffer_odata[0]);

	p( psi, pbi, pso, pbo );

	// Copy output position to vs_output.
	memset( &out.position, 0, sizeof(out.position) );
	storage_info* out_info = code->abii()->output_storage( semantic_value( sv_position ) );
	memcpy( &out.position, &(buffer_odata[out_info->offset]), out_info->size );
}

void vertex_shader_unit::set_variable( std::string const& name, void* data )
{
	storage_info* vsi = code->abii()->input_storage( name );
	memcpy( &buffer_data[vsi->offset], data, vsi->size );
}

vertex_shader_unit::vertex_shader_unit()
: code(NULL), sa(NULL)
{
}

vertex_shader_unit::vertex_shader_unit( vertex_shader_unit const& rhs )
	: code(rhs.code), sa(rhs.sa),
	stream_data(rhs.stream_data), buffer_data(rhs.buffer_data),
	stream_odata(rhs.stream_odata), buffer_odata(rhs.buffer_odata)
{
}

vertex_shader_unit& vertex_shader_unit::operator=( vertex_shader_unit const& rhs ){
	code = rhs.code;
	sa = rhs.sa;
	stream_data = rhs.stream_data;
	buffer_data = rhs.buffer_data;
	stream_odata = rhs.stream_odata;
	buffer_odata = rhs.buffer_odata;
	return *this;
}

vertex_shader_unit::~vertex_shader_unit()
{
}


END_NS_SALVIAR();