#ifndef SALVIA_SHADER_UNIT_H
#define SALVIA_SHADER_UNIT_H

#if defined( sasl_host_EXPORTS )
#define SALVIA_API __declspec( dllexport )
#else
#define SALVIA_API __declspec( dllimport )
#endif

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/platform/typedefs.h>

#include <vector>

BEGIN_NS_SALVIAR();

class shader_code;
class vs_output;
class stream_assembler;

class vertex_shader_unit
{
public:
	vertex_shader_unit();
	~vertex_shader_unit();

	vertex_shader_unit( vertex_shader_unit const& );
	vertex_shader_unit& operator = ( vertex_shader_unit const& );

	void initialize( shader_code const* );

	void bind_streams( stream_assembler const* sa );
	void set_variable( std::string const&, void* data );

	uint32_t output_attributes_count() const;
	uint32_t output_attribute_modifiers( size_t index ) const;

	void update( size_t ivert );

	void execute( vs_output& out );

public:
	shader_code const* code;
	stream_assembler const* sa;

	std::vector<char> stream_data;
	std::vector<char> buffer_data;

	std::vector<char> stream_odata;
	std::vector<char> buffer_odata;
};

class pixel_shader_unit
{
public:
	pixel_shader_unit();
	~pixel_shader_unit();

	pixel_shader_unit( pixel_shader_unit const& );
	pixel_shader_unit& operator = ( pixel_shader_unit const& );

	void initialize( shader_code const* );

	void update( vs_output* inputs );
	void execute( ps_output* outs );

public:
	shader_code const* code;

	std::vector<char> stream_data;
	std::vector<char> buffer_data;

	std::vector<char> stream_odata;
	std::vector<char> buffer_odata;
};

END_NS_SALVIAR();

#endif