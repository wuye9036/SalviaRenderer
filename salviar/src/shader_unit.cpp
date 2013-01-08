#include <salviar/include/shader_unit.h>

#include <salviar/include/shader_object.h>
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

using namespace eflib;
using std::vector;
using boost::shared_ptr;
using boost::shared_array;
using boost::make_shared;

void invoke( void* callee, void* psi, void* pbi, void* pso, void* pbo )
{
	reinterpret_cast<void (*)(void*, void*, void*, void*)>(callee)( psi, pbi, pso, pbo );
}

BEGIN_NS_SALVIAR();

void vertex_shader_unit::initialize( shader_object const* code ){
	this->code = code;
	this->stream_data.resize( code->get_reflection()->total_size( su_stream_in), 0 );
	this->buffer_data.resize( code->get_reflection()->total_size(su_buffer_in), 0 );
	this->stream_odata.resize( code->get_reflection()->total_size(su_stream_out), 0 );
	this->buffer_odata.resize( code->get_reflection()->total_size(su_buffer_out), 0 );
}

void vertex_shader_unit::bind_streams( stream_assembler const* sa ){
	this->sa = sa;
}

void vertex_shader_unit::update( size_t ivert )
{
	shader_reflection const* abii = code->get_reflection();
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
		= static_cast<void (*)(void*, void*, void*, void*)>( code->native_function() );

	void* psi = stream_data.empty() ? NULL : &(stream_data[0]);
	void* pbi = buffer_data.empty() ? NULL : &(buffer_data[0]);
	void* pso = stream_odata.empty() ? NULL : &(stream_odata[0]);
	void* pbo = buffer_odata.empty() ? NULL : &(buffer_odata[0]);

	invoke(p, psi, pbi, pso, pbo );

	// Copy output attributes to vs_output.
	// TODO: Semantic will be mapped.
	shader_reflection const* abii = code->get_reflection();
	vector<sv_layout*> infos = abii->layouts( su_buffer_out );

	size_t register_index = 0;
	BOOST_FOREACH( sv_layout* info, infos ){
		if( info->sv == semantic_value(sv_position) ){
			memset( &out.position, 0, sizeof(out.position) );
			memcpy( &out.position, &(buffer_odata[info->offset]), info->size );
		} else {
			memcpy( &out.attributes[register_index], &(buffer_odata[info->offset]), info->size );
			++register_index;
		}
	}
}

void vertex_shader_unit::set_variable( std::string const& name, void const* pvariable )
{
	sv_layout* vsi = code->get_reflection()->input_sv_layout( fixed_string(name) );
	EFLIB_ASSERT_AND_IF(vsi, "Cannot found variable.")
	{
		return;
	}
	memcpy( &buffer_data[vsi->offset], pvariable, vsi->size );
}

void vertex_shader_unit::set_variable_pointer( std::string const& name, void const* pvariable, size_t sz )
{
	shared_array<char> data_array(new char[sz]);
	memcpy(data_array.get(), pvariable, sz);
	dynamic_datas[name] = data_array;

	void* array_ptr[2] = {data_array.get(), data_array.get()};
	set_variable( name, &(array_ptr[0]) );
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
	// TODO: Need to be optimized.
	shader_reflection const* abii = code->get_reflection();
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

void vertex_shader_unit::set_sampler( std::string const& name, h_sampler const& samp )
{
	if( std::find( used_samplers.begin(), used_samplers.end(), samp ) != used_samplers.end() )
	{
		used_samplers.push_back(samp);
	}

	sampler* psamp = samp.get();
	set_variable( name, &psamp );
}

void pixel_shader_unit::initialize( shader_object const* code )
{
	this->code = code;
	size_t pixel_input_data_size = code->get_reflection()->total_size(su_stream_in);
	size_t pixel_output_data_size = code->get_reflection()->total_size(su_stream_out);

	size_t ps_input_size =
		PACKAGE_ELEMENT_COUNT * sizeof(void*) +
		PACKAGE_ELEMENT_COUNT * pixel_input_data_size;
	
	size_t ps_output_size =
		PACKAGE_ELEMENT_COUNT * sizeof(void*) +
		PACKAGE_ELEMENT_COUNT * pixel_output_data_size;

	this->stream_data.resize ( ps_input_size, 0 );
	this->buffer_data.resize ( code->get_reflection()->total_size(su_buffer_in ), 0 );
	this->stream_odata.resize( ps_output_size, 0 );
	this->buffer_odata.resize( code->get_reflection()->total_size(su_buffer_out), 0 );

	reset_pointers();
}

pixel_shader_unit::~pixel_shader_unit()
{
}

pixel_shader_unit::pixel_shader_unit() : code(NULL)
{
}

void pixel_shader_unit::reset_pointers()
{
	aligned_vector* streams[] = {&stream_data, &stream_odata};

	for(size_t i_stream = 0; i_stream < 2; ++i_stream)
	{
		aligned_vector& data_stream(*streams[i_stream]);

		void** pointer_start = reinterpret_cast<void**>( &(data_stream[0]) );
		size_t pointers_size = PACKAGE_ELEMENT_COUNT * sizeof(void*);
		size_t pixel_data_size = (data_stream.size() - pointers_size) / PACKAGE_ELEMENT_COUNT;
		for(size_t i_pixel = 0; i_pixel < PACKAGE_ELEMENT_COUNT; ++i_pixel)
		{
			void* ppixel = NULL;
			if(pixel_data_size > 0)
			{
				ppixel = &(data_stream[pointers_size+pixel_data_size*i_pixel]);
			}
			pointer_start[i_pixel] = ppixel;
		}
	}
}

pixel_shader_unit::pixel_shader_unit( pixel_shader_unit const& rhs )
	:  code(rhs.code),
	stream_data(rhs.stream_data), buffer_data(rhs.buffer_data),
	stream_odata(rhs.stream_odata), buffer_odata(rhs.buffer_odata)
{
	reset_pointers();
}

pixel_shader_unit& pixel_shader_unit::operator=( pixel_shader_unit const& rhs )
{
	code = rhs.code;
	stream_data = rhs.stream_data;
	buffer_data = rhs.buffer_data;
	stream_odata = rhs.stream_odata;
	buffer_odata = rhs.buffer_odata;

	reset_pointers();

	return *this;
}

void pixel_shader_unit::set_variable( std::string const& name, void const* data )
{
	sv_layout* vsi = code->get_reflection()->input_sv_layout(name);
	memcpy( &buffer_data[vsi->offset], data, vsi->size );
}

shared_ptr<pixel_shader_unit> pixel_shader_unit::clone() const
{
	if( this ){
		return make_shared<pixel_shader_unit>( *this );
	} else {
		return shared_ptr<pixel_shader_unit>();
	}
}

void pixel_shader_unit::update( vs_output* inputs, shader_reflection const* vs_abi )
{
	vector<sv_layout*> infos = code->get_reflection()->layouts( su_stream_in );

	size_t register_index = 0;

	BOOST_FOREACH( sv_layout* info, infos )
	{
		size_t pixel_data_size = info->total_size();
		if( info->sv == semantic_value(sv_position) )
		{
			for ( size_t i_pixel = 0; i_pixel < PACKAGE_ELEMENT_COUNT; ++i_pixel )
			{
				uintptr_t pixel_addr = * reinterpret_cast<uintptr_t*>( &(stream_data[i_pixel*sizeof(void*)]) );
				uintptr_t data_addr = pixel_addr + static_cast<uintptr_t>(info->offset);
				void* pdata = reinterpret_cast<void*>(data_addr);
				memset(pdata, 0, pixel_data_size);
				memcpy(pdata, &(inputs[i_pixel].position), info->size);
			}
		}
		else
		{
			size_t attr_index = 0;
			if(vs_abi){
				sv_layout* src_sv_layout = vs_abi->input_sv_layout( info->sv );
				attr_index = static_cast<size_t>( src_sv_layout->logical_index );
			} else {
				attr_index = register_index++;
			}

			for(size_t i_pixel = 0; i_pixel < PACKAGE_ELEMENT_COUNT; ++i_pixel)
			{
				uintptr_t pixel_addr = * reinterpret_cast<uintptr_t*>( &(stream_data[i_pixel*sizeof(void*)]) );
				uintptr_t data_addr = pixel_addr + static_cast<uintptr_t>(info->offset);
				void* pdata = reinterpret_cast<void*>(data_addr);

				memset(pdata, 0, pixel_data_size);
				memcpy(pdata, &(inputs[i_pixel].attributes[attr_index]), info->size);
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

	invoke( code->native_function(), psi, pbi, pso, pbo );

	shader_reflection const* abii = code->get_reflection();
	vector<sv_layout*> infos = abii->layouts( su_stream_out );

	BOOST_FOREACH(sv_layout* info, infos)
	{
		if( info->sv == semantic_value(sv_target) )
		{
			assert( info->value_type == lvt_f32v4 );
			for (size_t i_pixel = 0; i_pixel < PACKAGE_ELEMENT_COUNT; ++i_pixel)
			{
				uintptr_t pixel_addr = * reinterpret_cast<uintptr_t*>( &(stream_odata[i_pixel*sizeof(void*)]) );
				uintptr_t data_addr = pixel_addr + static_cast<uintptr_t>(info->offset);
				void* pdata = reinterpret_cast<void*>(data_addr);
				void* pbuffer = &(outs[i_pixel].color[info->sv.get_index()]);
				memcpy(pbuffer, pdata, info->size);
			}
		}
		else if( info->sv == semantic_value(sv_depth) )
		{
			for (size_t i_pixel = 0; i_pixel < PACKAGE_ELEMENT_COUNT; ++i_pixel)
			{
				uintptr_t pixel_addr = * reinterpret_cast<uintptr_t*>( &(stream_odata[i_pixel*sizeof(void*)]) );
				uintptr_t data_addr = pixel_addr + static_cast<uintptr_t>(info->offset);
				float* pdata = reinterpret_cast<float*>(data_addr);
				outs[i_pixel].depth = *pdata;
			}
		}
	}
}

void pixel_shader_unit::set_sampler( std::string const& name, h_sampler const& samp )
{
	if( std::find( used_samplers.begin(), used_samplers.end(), samp ) != used_samplers.end() )
	{
		used_samplers.push_back(samp);
	}

	sampler* psamp = samp.get();
	set_variable(name, &psamp);
}

END_NS_SALVIAR();