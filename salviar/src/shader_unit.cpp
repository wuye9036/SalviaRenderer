#include <salviar/include/shader_unit.h>

#include <salviar/include/shader_code.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/renderer.h>
#include <salviar/include/buffer.h>
#include <salviar/include/stream_assembler.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/math/math.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

using namespace sasl::semantic;
using namespace eflib;
using std::vector;
using boost::shared_ptr;
using boost::make_shared;

void invoke( void* callee, void* psi, void* pbi, void* pso, void* pbo )
{
#if defined(EFLIB_CPU_X86) && defined(EFLIB_MSVC)
	__asm{
		push ebp;

		push callee;

		push pbo;
		push pso;
		push pbi;
		push psi;

		mov  ebp, esp ;

		push ebx;
		push esi;
		push edi;

		and  esp, -16;
		sub  esp, 16;

		mov  ebx, [ebp+12];
		push ebx;
		mov  ebx, [ebp+8];
		push ebx;
		mov  ebx, [ebp+4];
		push ebx;
		mov  ebx, [ebp];
		push ebx;

		mov  ebx, [ebp+16];
		call ebx;

		mov  edi, [ebp-12];
		mov  esi, [ebp-8];
		mov  ebx, [ebp-4];
		mov  esp, ebp;
		add  esp, 20;
		pop  ebp;
	}

	// X XXXX
#else
	reinterpret_cast<void (*)(void*, void*, void*, void*)>(callee)( psi, pbi, pso, pbo );
#endif
}

BEGIN_NS_SALVIAR();

void vertex_shader_unit::initialize( shader_code const* code ){
	this->code = code;
	this->stream_data.resize( code->abii()->total_size( su_stream_in), 0 );
	this->buffer_data.resize( code->abii()->total_size(su_buffer_in), 0 );
	this->stream_odata.resize( code->abii()->total_size(su_stream_out), 0 );
	this->buffer_odata.resize( code->abii()->total_size(su_buffer_out), 0 );
}

void vertex_shader_unit::bind_streams( stream_assembler const* sa ){
	this->sa = sa;
}

void vertex_shader_unit::update( size_t ivert )
{
	shader_abi const* abii = code->abii();
	vector<sv_layout*> infos = abii->layouts( su_stream_in );

	BOOST_FOREACH( sv_layout* info, infos ){
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

#if defined(EFLIB_CPU_X86) && defined(EFLIB_MSVC)
	__asm{
		push ebp;

		push p;

		push pbo;
		push pso;
		push pbi;
		push psi;

		mov  ebp, esp ;

		push ebx;
		push esi;
		push edi;

		and  esp, -16;
		sub  esp, 16;

		mov  ebx, [ebp+12];
		push ebx;
		mov  ebx, [ebp+8];
		push ebx;
		mov  ebx, [ebp+4];
		push ebx;
		mov  ebx, [ebp];
		push ebx;

		mov  ebx, [ebp+16];
		call ebx;

		mov  edi, [ebp-12]
		mov  esi, [ebp-8]
		mov  ebx, [ebp-4]
		mov  esp, ebp
		add  esp, 20
		pop  ebp
	}

	// X XXXX
#else
	p( psi, pbi, pso, pbo );
#endif

	// Copy output attributes to vs_output.
	// TODO Semantic will be mapped.
	shader_abi const* abii = code->abii();
	vector<sv_layout*> infos = abii->layouts( su_buffer_out );

	size_t register_index = 0;
	BOOST_FOREACH( sv_layout* info, infos ){
		if( info->sv == semantic_value(sv_position) ){
			memset( &out.position, 0, sizeof(out.position) );
			memcpy( &out.position, &(buffer_odata[info->offset]), info->element_size );
		} else {
			memcpy( &out.attributes[register_index], &(buffer_odata[info->offset]), info->element_size );
			++register_index;
		}
	}
}

void vertex_shader_unit::set_variable( std::string const& name, void* data )
{
	sv_layout* vsi = code->abii()->input_sv_layout( name );
	memcpy( &buffer_data[vsi->offset], data, vsi->element_size );
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

uint32_t vertex_shader_unit::output_attributes_count() const{
	// TODO Need to be optimized.
	shader_abi const* abii = code->abii();
	vector<sv_layout*> infos = abii->layouts( su_buffer_out );

	uint32_t register_index = 0;
	BOOST_FOREACH( sv_layout* info, infos ){
		if( info->sv != semantic_value(sv_position) ){
			++register_index;
		}
	}
	return register_index;
}

uint32_t vertex_shader_unit::output_attribute_modifiers( size_t /*index*/ ) const{
	return vs_output::am_linear;
}

void pixel_shader_unit::initialize( shader_code const* code )
{
	this->code = code;
	this->stream_data.resize( code->abii()->total_size( su_stream_in), 0 );
	this->buffer_data.resize( code->abii()->total_size(su_buffer_in), 0 );
	this->stream_odata.resize( code->abii()->total_size(su_stream_out), 0 );
	this->buffer_odata.resize( code->abii()->total_size(su_buffer_out), 0 );
}

pixel_shader_unit::~pixel_shader_unit()
{
}

pixel_shader_unit::pixel_shader_unit() : code(NULL)
{
}

pixel_shader_unit::pixel_shader_unit( pixel_shader_unit const& rhs )
	:  code(rhs.code),
	stream_data(rhs.stream_data), buffer_data(rhs.buffer_data),
	stream_odata(rhs.stream_odata), buffer_odata(rhs.buffer_odata)
{

}

pixel_shader_unit& pixel_shader_unit::operator=( pixel_shader_unit const& rhs )
{
	code = rhs.code;
	stream_data = rhs.stream_data;
	buffer_data = rhs.buffer_data;
	stream_odata = rhs.stream_odata;
	buffer_odata = rhs.buffer_odata;

	return *this;
}

void pixel_shader_unit::set_variable( std::string const& name, void* data )
{
	sv_layout* vsi = code->abii()->input_sv_layout( name );
	memcpy( &buffer_data[vsi->offset], data, vsi->element_size );
}

shared_ptr<pixel_shader_unit> pixel_shader_unit::clone() const
{
	if( this ){
		return make_shared<pixel_shader_unit>( *this );
	} else {
		return shared_ptr<pixel_shader_unit>();
	}
}

void pixel_shader_unit::update( vs_output* inputs, shader_abi const* vs_abi )
{
	vector<sv_layout*> infos = code->abii()->layouts( su_stream_in );

	size_t register_index = 0;
	BOOST_FOREACH( sv_layout* info, infos ){
		int elem_stride = info->element_size + info->element_padding;
		if( info->sv == semantic_value(sv_position) ){
			for ( size_t i_elem = 0; i_elem < PACKAGE_ELEMENT_COUNT; ++i_elem ){
				void* pdata = &(stream_data[info->offset+elem_stride*i_elem]);
				memset( pdata, 0, elem_stride );
				memcpy( pdata, &( inputs[i_elem].position ), info->element_size );
			}
		} else {
			size_t attr_index = 0;
			if( vs_abi ){
				sv_layout* src_sv_layout = vs_abi->input_sv_layout( info->sv );
				attr_index = static_cast<size_t>( src_sv_layout->logical_index );
			} else {
				attr_index = register_index++;
			}

			for ( size_t i_elem = 0; i_elem < PACKAGE_ELEMENT_COUNT; ++i_elem ){
				void* pdata = &(stream_data[info->offset+elem_stride*i_elem]);
				memset( pdata, 0, elem_stride );
				memcpy( pdata, &( inputs[i_elem].attributes[attr_index] ), info->element_size );
			}
		}
	}
}

void pixel_shader_unit::execute( ps_output* outs )
{
	void* psi = stream_data.empty() ? NULL : &(stream_data[0]);
	void* pbi = buffer_data.empty() ? NULL : &(buffer_data[0]);
	void* pso = stream_odata.empty() ? NULL : &(stream_odata[0]);
	void* pbo = buffer_odata.empty() ? NULL : &(buffer_odata[0]);

	invoke( code->function_pointer(), psi, pbi, pso, pbo );

	shader_abi const* abii = code->abii();
	vector<sv_layout*> infos = abii->layouts( su_stream_out );

	BOOST_FOREACH( sv_layout* info, infos ){
		size_t elem_stride = info->element_size + info->element_padding;
		for ( size_t i_elem = 0; i_elem < PACKAGE_ELEMENT_COUNT; ++i_elem ){
			if( info->sv == semantic_value(sv_target) ){
				assert( info->value_type == lvt_f32v4 );
				for ( size_t i_elem = 0; i_elem < PACKAGE_ELEMENT_COUNT; ++i_elem ){
					void* pdata = &(stream_odata[info->offset+elem_stride*i_elem]);
					memcpy( &( outs[i_elem].color[info->sv.get_index()] ), pdata, info->element_size );
				}
			} else if( info->sv == semantic_value(sv_depth) ){
				float* pdata = (float*)( &(stream_odata[info->offset+elem_stride*i_elem]) );
				outs[i_elem].depth = *pdata;
			}
		}
	}
}

void pixel_shader_unit::set_sampler( std::string const& name, h_sampler const& samp )
{
	if( std::find( used_samplers.begin(), used_samplers.end(), samp ) != used_samplers.end() ){
		used_samplers.push_back(samp);
	}

	sampler* psamp = samp.get();
	set_variable( name, &psamp );
}

END_NS_SALVIAR();