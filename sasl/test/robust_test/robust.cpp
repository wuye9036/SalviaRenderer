#define ALL_TESTS_ENABLED 1

#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sasl/include/driver/driver_api.h>
#include <sasl/include/common/diag_formatter.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <salviar/include/shader_abi.h>

#include <eflib/include/math/vector.h>
#include <eflib/include/math/matrix.h>
#include <eflib/include/metaprog/util.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/platform/cpuinfo.h>

#include <fstream>
#include <iostream>

using namespace eflib;

using sasl::driver::driver;
using sasl::code_generator::jit_engine;
using sasl::code_generator::llvm_module;
using sasl::common::diag_chat;
using sasl::common::diag_item;
using sasl::semantic::symbol;

using salviar::PACKAGE_ELEMENT_COUNT;
using salviar::PACKAGE_LINE_ELEMENT_COUNT;

using boost::shared_ptr;
using boost::shared_polymorphic_cast;
using boost::format;
namespace fs = boost::filesystem;

using std::fstream;
using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::pair;
using std::make_pair;

BOOST_AUTO_TEST_SUITE( robust )

string make_command( string const& file_name, string const& options)
{
	return ( format("--input=\"%s\" %s") % file_name % options ).str();
}

string make_command( string const& inc, string const& sysinc, string const& file_name, string const& options )
{
	return ( format("--input=\"%s\" -I \"%s\" -S \"%s\" %s") % file_name % inc % sysinc % options ).str();
}

bool print_diagnostic( diag_chat*, diag_item* item )
{
	BOOST_MESSAGE( sasl::common::str(item) );
	return true;
}


struct jit_fixture {
	jit_fixture() {}

	void init_g( string const& file_name ){
		init( file_name, "--lang=g" );
	}

	void init_vs( string const& file_name ){
		init( file_name, "--lang=vs" );
	}

	void init_ps( string const& file_name ){
		init( file_name, "--lang=ps" );
	}

	void add_virtual_file( char const* name, char const* content )
	{
		vfiles.push_back( make_pair(name, content) );
	}

	void init( string const& file_name, string const& options ){
		init_cmd( make_command(file_name, options), file_name );
	}

	void init_cmd( string const& cmd, string const& dump_file_name )
	{
		diags = diag_chat::create();
		diags->add_report_raised_handler( print_diagnostic );
		sasl_create_driver(drv);
		BOOST_REQUIRE(drv);
		drv->set_parameter(cmd);
		for( size_t i = 0; i < vfiles.size(); ++i )
		{
			drv->add_virtual_file( vfiles[i].first, vfiles[i].second, true );
		}

		shared_ptr<diag_chat> results = drv->compile();
		diag_chat::merge(diags.get(), results.get(), true);

		BOOST_REQUIRE( drv->root() );
		BOOST_REQUIRE( drv->mod_si() );
		BOOST_REQUIRE( drv->mod_codegen() );

		root_sym = drv->mod_si()->root();

		shared_ptr<llvm_module> llvm_mod = shared_polymorphic_cast<llvm_module>( drv->mod_codegen() );
		fstream dump_file( ( dump_file_name + "_ir.ll" ).c_str(), std::ios::out );
		llvm_mod->dump( dump_file );
		dump_file.close();

		je = drv->create_jit();
		BOOST_REQUIRE( je );
	}
	~jit_fixture(){}

	shared_ptr<driver>		drv;
	shared_ptr<symbol>		root_sym;
	shared_ptr<jit_engine>	je;
	shared_ptr<diag_chat>	diags;
	vector< pair<char const*, char const*> > vfiles;
};

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( incomplete, jit_fixture ){
	init_g( "./repo/question/v1a1/incomplete.ss" );
}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( semantic_errors, jit_fixture )
{
	init_g( "./repo/question/v1a1/semantic_errors.ss" );
}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( include_test, jit_fixture ){
	const char* virtual_include_content = 
		"float virtual_include_add(float a, float b){ \r\n"
		"	return a+b; \r\n"
		"} \r\n";
	add_virtual_file( "virtual_include.ss", virtual_include_content );
	init_g( "./repo/question/v1a1/include_main.ss" );
}
#endif

#if 1 || ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( include_dir_test, jit_fixture ){
	fs::path include_path = fs::current_path() / "./repo/question/v1a1/include";
	fs::path sysincl_path = fs::current_path() / "./repo/question/v1a1/sysincl";

	BOOST_REQUIRE( fs::is_directory(include_path) && fs::is_directory(sysincl_path) );

	std::string file_name(".\\repo\\question/v1a1/include_search_path.ss");
	std::string cmd = make_command( include_path.string(), sysincl_path.string(), file_name, "--lang=g" );
	init_cmd( cmd, file_name );
}
#endif

#if 1 || ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( include_dir_test_need_failed, jit_fixture ){
	fs::path include_path = fs::current_path() / "./repo/question/v1a1/include";
	fs::path sysincl_path = fs::current_path() / "./repo/question/v1a1/sysincl";

	BOOST_REQUIRE( fs::is_directory(include_path) && fs::is_directory(sysincl_path) );

	std::string file_name(".\\repo\\question/v1a1/include_search_path.ss");
	std::string cmd = make_command( sysincl_path.string(),  include_path.string(), file_name, "--lang=g" );
	init_cmd( cmd, file_name );
}
#endif

BOOST_AUTO_TEST_SUITE_END();
