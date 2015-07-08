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
	shared_ptr<diag_chat> results = drv->compile(true, true);
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
}

namespace 
{
	struct X {};
	
	void jit_function_compiling_testing()
	{
		typedef if_< or_< is_arithmetic<_>, is_pointer<_> >, _, add_reference<_> > Conv;

		static_assert(is_same<make_function<void, type_list<int>>::type, void(int)>::value,	  "Make Function0 Assertion Failed");
		static_assert(is_same<make_function<void, type_list<>>::type,    void()>::value,	  "Make Function1 Assertion Failed");
		static_assert(is_same<make_function<int , type_list<>>::type,    void(int)>::value,	  "Make Function2 Assertion Failed");

		static_assert(is_same<boost::mpl::apply<Conv, int>::type, int>::value,                "Applier0 Assertion Failed");
		static_assert(is_same<boost::mpl::apply<Conv, X>::type,   X& >::value,                "Applier1 Assertion Failed");

		static_assert(is_same<convert_types<Conv, int>::type,	  type_list<int>>::value,     "Convert Types0 Assertion Failed");
		static_assert(is_same<convert_types<Conv, X>::type,		  type_list<X&>>::value,	  "Convert Types1 Assertion Failed");
		static_assert(is_same<convert_types<Conv, int, X>::type,  type_list<int, X&>>::value, "Convert Types2 Assertion Failed");

		typedef convert_to_jit_function_type<Conv, void(int)> convertor0;
		static_assert(is_same<convertor0::type,        void(int)>::value, "Convertor0 Type Assertion Failed");
		static_assert(is_same<convertor0::return_type, void     >::value, "Convertor0 Return Type Assertion Failed");

		typedef convert_to_jit_function_type<Conv, int(int)> convertor1;
		static_assert(is_same<convertor1::type,        void(int*, int)>::value, "Convertor1 Type Assertion Failed");
		static_assert(is_same<convertor1::return_type, int            >::value, "Convertor1 Return Type Assertion Failed");

		typedef convert_to_jit_function_type<Conv, X (int, X, X*, float, double*)> convertor2;
		static_assert(is_same<convertor2::type,        void(X*, int, X&, X*, float, double*)>::value, "Convertor2 Type Assertion Failed");
		static_assert(is_same<convertor2::return_type, X                >::value, "Convertor2 Return Type Assertion Failed");
		
		typedef convert_to_jit_function_type<Conv, int()> convertor3;
		static_assert(is_same<convertor3::type,        void(int*)>::value, "Convertor3 Type Assertion Failed");
		static_assert(is_same<convertor3::return_type, int       >::value, "Convertor3 Return Type Assertion Failed");

		jit_function< int(int) > jit_func0;
		jit_function< int(void)> jit_func1;

		static_assert( is_same<jit_function<int(int)>::callee_type, void(*)(int*, int)>::value, "Callee Type 0 Assertion Failed");
		static_assert( is_same<jit_function<int()>::callee_type,    void(*)(int*)>::value,      "Callee Type 1 Assertion Failed");

		jit_func0(0);
		jit_func1();
	}
}
