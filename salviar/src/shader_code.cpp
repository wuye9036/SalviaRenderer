#include <salviar/include/shader_code.h>

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

shared_ptr<shader_code> shader_code::create( std::string const& code, salviar::languages lang, vector<string>& results )
{
	std::string dll_name = "sasl_host";
#ifdef EFLIB_DEBUG
	dll_name += "_d";
#endif
	dll_name += ".dll";

	shared_ptr<dynamic_lib> dl = dynamic_lib::load( dll_name );
	typedef vector< tuple<void*, string, bool> > external_function_array;
	void (*create_shader_code)(
		shared_ptr<shader_code>&,
		string const&,
		salviar::languages,
		external_function_array const&,
		boost::shared_ptr< vector<string> >& );

	dl->get_function( create_shader_code, std::string("salvia_create_shader") );

	external_function_array extfns;
	extfns.push_back( make_tuple(&salviar_tex2Dlod,		  "sasl.vs.tex2d.lod" ,  true ) );
	extfns.push_back( make_tuple(&salviar_texCUBElod,	  "sasl.vs.texCUBE.lod", true ) );
	extfns.push_back( make_tuple(&salviar_tex2Dlod_pkg,	  "sasl.ps.tex2d.lod" ,  true ) );
	extfns.push_back( make_tuple(&salviar_tex2Dgrad_pkg,  "sasl.ps.tex2d.grad",  true ) );
	extfns.push_back( make_tuple(&salviar_tex2Dbias_pkg,  "sasl.ps.tex2d.bias",  true ) );
	extfns.push_back( make_tuple(&salviar_tex2Dproj_pkg,  "sasl.ps.tex2d.proj",  true ) );
	shared_ptr<shader_code> ret;
	shared_ptr< vector<string> > presults;
	create_shader_code( ret, code, lang, extfns, presults );
	results = *presults;
	if(ret)
	{
		ret->update_native_function();
	}
	
	return ret;
}

shared_ptr<shader_code> shader_code::create_and_log( string const& code, salviar::languages lang )
{
	vector<string> logs;
	shared_ptr<shader_code> ret = shader_code::create( code, lang, logs );
	if(!ret){
		cout << "Shader was compiled failed!" << endl;
		for( size_t i = 0; i < logs.size(); ++i ){
			cout << logs[i] << endl;
		}
	}

	return ret;
}

END_NS_SALVIAR();

