#include <salviar/include/shader_unit.h>
#include <salviar/include/renderer.h>

#include <sasl/include/semantic/abi_info.h>

#include <eflib/include/diagnostics/assert.h>

using namespace sasl::semantic;
using std::vector;

BEGIN_NS_SALVIAR();

void vertex_shader_unit::initialize( shader_code const* code ){
	this->code = code;
}

void vertex_shader_unit::bind_streams( vector<input_element_decl> const& layout ){
	this->layout = layout;
}

void vertex_shader_unit::update( size_t ivert )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
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