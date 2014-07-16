#include <sasl/test/jit_test/jit_test.h>

#include <sasl/include/drivers/drivers_api.h>
#include <sasl/include/codegen/cg_api.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/common/diag_formatter.h>
#include <sasl/include/common/diag_chat.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <fstream>

using sasl::codegen::module_vmcode;
using std::fstream;

#include <eflib/include/platform/disable_warnings.h>
void invoke( void* callee, void* psi, void* pbi, void* pso, void* pbo )
{
	reinterpret_cast<void (*)(void*, void*, void*, void*)>(callee)( psi, pbi, pso, pbo );
}

string make_command( string const& file_name, string const& options )
{
	return "--input=\"" + file_name + "\" " + options;
}

bool print_diagnostic( diag_chat*, diag_item* item )
{
	BOOST_MESSAGE( sasl::common::str(item) );
	return true;
}

#include <eflib/include/platform/enable_warnings.h>

void jit_fixture::init_g( string const& file_name )
{
	init( file_name, "--lang=g" );
}

void jit_fixture::init_vs( string const& file_name )
{
	init( file_name, "--lang=vs" );
}

void jit_fixture::init_ps( string const& file_name )
{
	init( file_name, "--lang=ps" );
}

void* jit_fixture::function( string const& unmangled_name )
{
	assert( !root_sym->find_overloads(unmangled_name).empty() );
	string fn_name = root_sym->find_overloads(unmangled_name)[0]->mangled_name();
	return vmc->get_function(fn_name);
}

void jit_fixture::set_function( void* fn, string const& unmangled_name )
{
	assert( !root_sym->find_overloads(unmangled_name).empty() );
	string fn_name = root_sym->find_overloads(unmangled_name)[0]->mangled_name();
	vmc->inject_function(fn, fn_name);
}

void jit_fixture::set_raw_function( void* fn, string const& mangled_name )
{
	vmc->inject_function(fn, mangled_name);
}

void jit_fixture::init(string const& file_name, string const& options)
{
	diags = diag_chat::create();
	diags->add_report_raised_handler( print_diagnostic );
	sasl_create_compiler(drv);
	BOOST_REQUIRE(drv);
	drv->set_parameter( make_command(file_name, options) );
	shared_ptr<diag_chat> results = drv->compile(true);
	diag_chat::merge(diags.get(), results.get(), true);

	if( !drv->get_root() || !drv->get_semantic() || !drv->get_vmcode() )
	{
		for(auto const& diag_item: diags->diag_items())
		{
			BOOST_ERROR( sasl::common::str(diag_item).c_str() );
		}
	}

	BOOST_REQUIRE( drv->get_root() );
	BOOST_REQUIRE( drv->get_semantic() );
	BOOST_REQUIRE( drv->get_vmcode() );

	root_sym = drv->get_semantic()->root_symbol();

	vmc = drv->get_vmcode();
	fstream dump_file( (file_name + "_ir.ll").c_str(), std::ios::out );
	vmc->dump_ir(dump_file);
	dump_file.close();

	bool is_jit_enabled = vmc->enable_jit();
	BOOST_REQUIRE(is_jit_enabled);
}
