#include <sasl/include/host/host.h>

#include <sasl/include/common/diag_chat.h>
#include <sasl/include/common/diag_item.h>
#include <sasl/include/common/diag_formatter.h>
#include <sasl/include/driver/driver_api.h>
#include <sasl/include/codegen/jit_api.h>
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
using boost::make_shared;

using namespace salviar;
using namespace sasl::codegen;
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
	vector< tuple<void*, string, bool> > const& ext_fns, shared_ptr< vector<string> >& diags )
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
	shared_ptr<diag_chat> results = drv->compile();

	diags = make_shared< vector<string> >();
	for(size_t i = 0; i < results->diag_items().size(); ++i)
	{
		diags->push_back( sasl::common::str(results->diag_items()[i]) );
	}
	
	shared_ptr<shader_code_impl> ret( new shader_code_impl() );
	
	shared_ptr<abi_info> abinfo = drv->mod_abi();
	if(!abinfo){ return; }
	ret->abii(abinfo);

	shared_ptr<jit_engine> je = drv->create_jit(ext_fns);
	if(!je){ return; }
	ret->jit( je );
	
	scode = ret;
}

SASL_HOST_API void salvia_initialize_host()
{
	sasl_initialize_driver();
}

SASL_HOST_API void salvia_finalize_host()
{
	sasl_finalize_driver();
}
