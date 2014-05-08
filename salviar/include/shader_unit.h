#pragma once

#if defined( sasl_host_EXPORTS )
#define SALVIA_API __declspec( dllexport )
#else
#define SALVIA_API __declspec( dllimport )
#endif

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/memory/allocator.h>
#include <eflib/include/utility/shared_declaration.h>

#include <vector>

BEGIN_NS_SALVIAR();

class  shader_reflection;
class  shader_object;
class  vs_output;
struct ps_output;
class  stream_assembler;

class vertex_shader_unit
{
public:
	vertex_shader_unit();
	~vertex_shader_unit();

	vertex_shader_unit( vertex_shader_unit const& );
	vertex_shader_unit& operator = ( vertex_shader_unit const& );

	void initialize( shader_object const* );

	void bind_streams( stream_assembler const* sa );
	void set_variable( std::string const&, void const* pvariable );
	void set_variable_pointer( std::string const&, void const* pvariable, size_t sz);
	void set_sampler( std::string const&, sampler_ptr const& samp );

	uint32_t output_attributes_count() const;
	uint32_t output_attribute_modifiers( size_t index ) const;

	void update( size_t ivert );

	void execute( vs_output& out );

public:
	shader_object const*		code;
	stream_assembler const*		sa;

	std::vector<sampler_ptr>	used_samplers;	// For take ownership

	std::vector<char>			stream_data;
	std::vector<char>			buffer_data;

	std::vector<char>			stream_odata;
	std::vector<char>			buffer_odata;

	boost::unordered_map<
		std::string,
		boost::shared_array<char> 
	>							dynamic_datas;
};

class pixel_shader_unit
{
public:
	pixel_shader_unit();
	~pixel_shader_unit();

	pixel_shader_unit( pixel_shader_unit const& );
	pixel_shader_unit& operator = ( pixel_shader_unit const& );
	
	boost::shared_ptr<pixel_shader_unit> clone() const;

	void initialize( shader_object const* );
	void reset_pointers();

	void set_variable( std::string const&, void const* data );
	void set_sampler( std::string const&, sampler_ptr const& samp );

	void update( vs_output* inputs, shader_reflection const* vs_abi );
	void execute(ps_output* outs, float* depths);

public:
	shader_object const* code;

	std::vector<sampler_ptr>									used_samplers;	// For take ownership

	typedef std::vector<char, eflib::aligned_allocator<char, 32> > aligned_vector;

	aligned_vector stream_data;
	aligned_vector buffer_data;

	aligned_vector stream_odata;
	aligned_vector buffer_odata;
};

EFLIB_DECLARE_CLASS_SHARED_PTR(vx_shader_unit);
class vx_shader_unit
{
public:
	virtual uint32_t output_attributes_count() const = 0;
	virtual uint32_t output_attribute_modifiers(size_t index) const = 0;

	virtual void execute(size_t ivert, void* out_data) = 0;
	virtual void execute(size_t ivert, vs_output& out) = 0;

	virtual ~vx_shader_unit(){}
};

END_NS_SALVIAR();