#include <salviar/include/shader_object.h>

#include <salviar/include/sampler_api.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/tuple/tuple.hpp>
#include <eflib/include/platform/boost_end.h>

#include <iostream>
#include <fstream>

using boost::shared_ptr;
using boost::tuple;
using boost::make_tuple;

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::fstream;

BEGIN_NS_SALVIAR();

/*
void (*host_module::sasl_initialize_host)();
void (*host_module::sasl_finalize_host)();
void (*host_module::create_shader_code)(
	shared_ptr<shader_object>&,
	string const&,
	salviar::languages,
	host_module::external_function_array const&,
	boost::shared_ptr< vector<string> >& );
shared_ptr<dynamic_lib> host_module::lib_;

class auto_init_host
{
public:
	auto_init_host(){ host_module::initialize();  }
	~auto_init_host(){ host_module::finalize(); }
} auto_init_host_obj;
*/

END_NS_SALVIAR();

