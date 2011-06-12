#ifndef SALVIA_SHADER_UNIT_H
#define SALVIA_SHADER_UNIT_H

#if defined( sasl_host_EXPORTS )
#define SALVIA_API __declspec( dllexport )
#else
#define SALVIA_API __declspec( dllimport )
#endif

#include <salviar/include/salviar_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SALVIAR();

class shader_code;
class compiler;

class vertex_shader_unit
{
public:
	void initialize( shader_code const* );

	void bind_streams( input_layout_decl const& layout );
	void set_variable( std::string const&, void* data );

	void update( size_t ivert );

	void execute();

public:
	shader_code const* code;
	std::vector<char> data;
};

class pixel_shader_unit
{
private:
	void initialize( shader_code const* );

	void update();

	void execute();

private:
	shader_code const* code;
	std::vector<char> data;
};

//extern "C"{
//	SALVIA_API void salvia_create_shader_units(
//		boost::shared_ptr<vertex_shader_unit>&,
//		boost::shared_ptr<pixel_shader_unit>&,
//		shader_code const*
//		);
//
//	SALVIA_API boost::shared_ptr<vertex_shader_unit> salvia_create_vs_unit( shader_code const* );
//	SALVIA_API boost::shared_ptr<pixel_shader_unit> salvia_create_ps_unit( shader_code const* );
//};

END_NS_SALVIAR();

#endif