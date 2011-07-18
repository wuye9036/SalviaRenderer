#include <salviar/include/shader_code.h>

#include <iostream>
#include <fstream>

using boost::shared_ptr;
using std::cout;
using std::endl;
using std::fstream;

BEGIN_NS_SALVIAR();

shared_ptr<shader_code> shader_code::create( std::string const& code, salviar::languages lang )
{
	std::_tstring dll_name = _EFLIB_T("sasl_host");
#ifdef EFLIB_DEBUG
	dll_name += _EFLIB_T("_d");
#endif
	dll_name += _EFLIB_T(".dll");

	shared_ptr<dynamic_lib> dl( dll_name );
	void (*create_shader_code)( shared_ptr<shader_code>&, std::string const&, salviar::languages );
	dl->get_function( create_shader_code, "salvia_create_shader_code" );

	shared_ptr<shader_code> ret;
	create_shader_code( ret, code, lang );

	return ret;
}

END_NS_SALVIAR();

