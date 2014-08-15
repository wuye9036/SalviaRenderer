#pragma once

#include <sasl/include/drivers/compiler.h>
#include <sasl/include/drivers/drivers_api.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/common/diag_chat.h>
#include <sasl/include/common/diag_item.h>
#include <sasl/include/common/diag_formatter.h>

#include <eflib/include/math/vector.h>
#include <eflib/include/math/matrix.h>
#include <eflib/include/utility/operator_bool.h>
#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/platform/dl_loader.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/test_tools.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

#if defined(EFLIB_WINDOWS)
#include <excpt.h>
#endif

namespace sasl
{
	namespace semantic
	{
		class symbol;
	}
	namespace codegen
	{
		EFLIB_DECLARE_CLASS_SHARED_PTR(module_vmcode);
	}
	namespace common
	{
		class diag_chat;
		class diag_item;
	}
}

using sasl::drivers::compiler;
using sasl::common::diag_chat;
using sasl::common::diag_item;
using sasl::common::diag_levels;
using sasl::semantic::symbol;

using eflib::vector_;
using eflib::matrix_;

using boost::format;
using boost::shared_ptr;

using std::string;
using std::vector;
using std::pair;
using std::make_pair;

struct compiler_loader
{
	compiler_loader()
	{
		lib_ = eflib::dynamic_lib::load( EFLIB_DYNAMIC_LIB_NAME("sasl_drivers") );
		lib_->get_function( create_compiler, "sasl_create_compiler" );
	}

	static void (*create_compiler)( shared_ptr<compiler>& );
private:
	boost::shared_ptr<eflib::dynamic_lib> lib_;
};

string make_command(string const& file_name, string const& options)
{
	return (format("--input=\"%s\" %s") % file_name % options).str();
}

string make_command( string const& inc, string const& sysinc, string const& file_name, string const& options )
{
	return ( format("--input=\"%s\" -I \"%s\" -S \"%s\" %s") % file_name % inc % sysinc % options ).str();
}

bool print_diagnostic( diag_chat*, diag_item* item )
{
	switch( item->level() )
	{
	case diag_levels::dl_error:
	case diag_levels::dl_fatal_error:
		BOOST_MESSAGE( sasl::common::str(item).raw_string() );
		break;
	default:
		// BOOST_MESSAGE( sasl::common::str(item).raw_string() );
		break;
	}
	
	return true;
}

struct abi_test_fixture 
{
	abi_test_fixture() {}

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
		init_cmd( make_command(file_name, options) );
	}

	void init_cmd(string const& cmd)
	{
		diags = diag_chat::create();
		diags->add_report_raised_handler( print_diagnostic );
		compiler_loader::create_compiler(drv);
		BOOST_REQUIRE(drv);
		drv->set_parameter(cmd);
		for( size_t i = 0; i < vfiles.size(); ++i )
		{
			drv->add_virtual_file( vfiles[i].first, vfiles[i].second, true );
		}

		shared_ptr<diag_chat> results = drv->compile(false, true);
		diag_chat::merge(diags.get(), results.get(), true);

		BOOST_REQUIRE( drv->get_root() );
		BOOST_REQUIRE( drv->get_semantic() );

		root_sym = drv->get_semantic()->root_symbol();
	}
	~abi_test_fixture(){}

	shared_ptr<compiler>						drv;
	symbol*										root_sym;
	shared_ptr<diag_chat>						diags;
	vector< pair<char const*, char const*> >	vfiles;
};