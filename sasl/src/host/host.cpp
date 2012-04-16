#include <sasl/include/host/host.h>

#include <sasl/include/driver/driver_api.h>
#include <sasl/include/code_generator/jit_api.h>
#include <sasl/include/semantic/abi_info.h>

#include <fstream>

using boost::shared_ptr;

using std::cout;
using std::endl;
using std::fstream;
using std::string;
using std::vector;

using boost::shared_static_cast;
using boost::shared_dynamic_cast;
using boost::tuple;

using namespace salviar;
using namespace sasl::code_generator;
using namespace sasl::common;
using namespace sasl::syntax_tree;
using namespace sasl::host;
using namespace sasl::semantic;

using sasl::driver::driver;

BEGIN_NS_SASL_HOST();

shader_code_impl::shader_code_impl(): pfn(NULL){
}

void shader_code_impl::abii( shared_ptr<shader_abi> const& v ){
	abi = shared_dynamic_cast<abi_info>( v );
}

shader_abi const* shader_code_impl::abii() const{
	return abi.get();
}

void shader_code_impl::jit( shared_ptr<jit_engine> const& v ){
	je = v;
}

void* shader_code_impl::function_pointer() const{
	return pfn;
}

void shader_code_impl::update_native_function(){
	assert( abi && je );
	if( !abi || !je ){
		pfn = NULL;
		return;
	}
	pfn = je->get_function( abi->entry_name() );
}

END_NS_SASL_HOST();

void salvia_create_shader(
	shared_ptr<salviar::shader_code>& scode, string const& code, languages lang,
	vector< tuple<void*, string, bool> > const& ext_fns )
{
	shared_ptr<driver> drv;
	sasl_create_driver(drv);

	drv->set_code(code);

	const char* lang_name = NULL;
	switch(lang)
	{
	case lang_pixel_shader:
		lang_name = "--lang=ps";
		break;
	case lang_vertex_shader:
		lang_name = "--lang=vs";
		break;
	default:
		lang_name = "--lang=g";
		break;
	}

	drv->set_parameter(lang_name);
	drv->compile();

	shared_ptr<shader_code_impl> ret( new shader_code_impl() );
	ret->abii( drv->mod_abi() );
	ret->jit( drv->create_jit(ext_fns) );
	
	scode = ret;
}