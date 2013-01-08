#include <salviar/include/shader_object.h>

#include <salviar/include/sampler_api.h>
#include <eflib/include/platform/dl_loader.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/tuple/tuple.hpp>
#include <eflib/include/platform/boost_end.h>

#include <iostream>
#include <fstream>

using eflib::dynamic_lib;

using boost::shared_ptr;
using boost::tuple;
using boost::make_tuple;

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::fstream;

BEGIN_NS_SALVIAR();

class host_module
{
public:
	static void initialize()
	{
		assert(!lib_);
		std::string dll_name = "sasl_host";
#ifdef EFLIB_DEBUG
		dll_name += "_d";
#endif
		dll_name += ".dll";

		// Load DLL
		lib_ = dynamic_lib::load(dll_name);

		// Fetch function pointers.
		lib_->get_function( sasl_initialize_host, std::string("salvia_initialize_host") );
		lib_->get_function( create_shader_code, std::string("salvia_create_shader") );
		lib_->get_function( sasl_finalize_host, std::string("salvia_finalize_host") );

		// Initialize library.
		sasl_initialize_host();
	}

	static void finalize()
	{
		assert(lib_);
		sasl_finalize_host();
		lib_.reset();
	}

	typedef vector< tuple<void*, string, bool> > external_function_array;
	static void (*create_shader_code)(
		shared_ptr<shader_object>&,
		string const&,
		salviar::languages,
		external_function_array const&,
		boost::shared_ptr< vector<string> >& );
private:
	static void (*sasl_initialize_host)();
	static void (*sasl_finalize_host)();
	static shared_ptr<dynamic_lib> lib_;
};

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

/*
shader_object_ptr shader_object::create(
	std::string const& code, salviar::languages lang, shader_log_ptr& log
	)
{
	host_module::external_function_array extfns;
	extfns.push_back( make_tuple(&salviar_tex2Dlod,		  "sasl.vs.tex2d.lod" ,  true ) );
	extfns.push_back( make_tuple(&salviar_texCUBElod,	  "sasl.vs.texCUBE.lod", true ) );
	extfns.push_back( make_tuple(&salviar_tex2Dlod_pkg,	  "sasl.ps.tex2d.lod" ,  true ) );
	extfns.push_back( make_tuple(&salviar_tex2Dgrad_pkg,  "sasl.ps.tex2d.grad",  true ) );
	extfns.push_back( make_tuple(&salviar_tex2Dbias_pkg,  "sasl.ps.tex2d.bias",  true ) );
	extfns.push_back( make_tuple(&salviar_tex2Dproj_pkg,  "sasl.ps.tex2d.proj",  true ) );
	shared_ptr<shader_object> ret;
	shared_ptr< vector<string> > presults;
	host_module::create_shader_code( ret, code, lang, extfns, presults );
	results = *presults;
	if(ret)
	{
		ret->update_native_function();
	}
	
	return ret;
}

shared_ptr<shader_object> compile( string const& code, salviar::languages lang )
{
	vector<string> logs;
	shared_ptr<shader_object> ret = shader_object::create( code, lang, logs );
	if(!ret){
		cout << "Shader was compiled failed!" << endl;
		for( size_t i = 0; i < logs.size(); ++i ){
			cout << logs[i] << endl;
		}
	}

	return ret;
}

void shader_object::initialize()
{
	host_module::initialize();
}

void shader_object::finalize()
{
	host_module::finalize();
}
*/
END_NS_SALVIAR();

