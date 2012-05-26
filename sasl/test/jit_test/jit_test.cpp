#include <sasl/test/jit_test/jit_test.h>

#include <sasl/include/driver/driver_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/common/diag_formatter.h>
#include <sasl/include/common/diag_chat.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <fstream>

using sasl::code_generator::llvm_module;
using std::fstream;

#include <eflib/include/platform/disable_warnings.h>
void invoke( void* callee, void* psi, void* pbi, void* pso, void* pbo )
{
#if defined(EFLIB_CPU_X86) && defined(EFLIB_MSVC)
	__asm{
		push ebp;

		push callee;

		push pbo;
		push pso;
		push pbi;
		push psi;

		mov  ebp, esp ;

		push ebx;
		push esi;
		push edi;

		and  esp, -16;
		sub  esp, 16;

		mov  ebx, [ebp+12];
		push ebx;
		mov  ebx, [ebp+8];
		push ebx;
		mov  ebx, [ebp+4];
		push ebx;
		mov  ebx, [ebp];
		push ebx;

		mov  ebx, [ebp+16];
		call ebx;

		mov  edi, [ebp-12];
		mov  esi, [ebp-8];
		mov  ebx, [ebp-4];
		mov  esp, ebp;
		add  esp, 20;
		pop  ebp;
	}
#else
	reinterpret_cast<void (*)(void*, void*, void*, void*)>(callee)( psi, pbi, pso, pbo );
#endif
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
	return je->get_function(fn_name);
}

void jit_fixture::set_function( void* fn, string const& unmangled_name )
{
	assert( !root_sym->find_overloads(unmangled_name).empty() );
	string fn_name = root_sym->find_overloads(unmangled_name)[0]->mangled_name();
	je->inject_function( fn, fn_name );
}

void jit_fixture::set_raw_function( void* fn, string const& mangled_name )
{
	je->inject_function(fn, mangled_name);
}

void jit_fixture::init( string const& file_name, string const& options )
{
	diags = diag_chat::create();
	diags->add_report_raised_handler( print_diagnostic );
	sasl_create_driver(drv);
	BOOST_REQUIRE(drv);
	drv->set_parameter( make_command(file_name, options) );
	shared_ptr<diag_chat> results = drv->compile();
	diag_chat::merge(diags.get(), results.get(), true);

	BOOST_REQUIRE( drv->root() );
	BOOST_REQUIRE( drv->mod_si() );
	BOOST_REQUIRE( drv->mod_codegen() );

	root_sym = drv->mod_si()->root();

	shared_ptr<llvm_module> llvm_mod = shared_polymorphic_cast<llvm_module>( drv->mod_codegen() );
	fstream dump_file( ( file_name + "_ir.ll" ).c_str(), std::ios::out );
	llvm_mod->dump( dump_file );
	dump_file.close();

	je = drv->create_jit();
	BOOST_REQUIRE( je );
}
