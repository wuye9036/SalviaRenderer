#include <sasl/include/driver/driver_impl.h>

#include <sasl/include/driver/code_sources.h>
#include <sasl/include/driver/options.h>

#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>
#include <sasl/include/semantic/semantic_api.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/parser/parse_api.h>
#include <sasl/include/parser/diags.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/common/diag_chat.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <fstream>

namespace po = boost::program_options;

using sasl::code_generator::llvm_module;
using sasl::code_generator::generate_llvm_code;
using sasl::code_generator::codegen_context;
using sasl::code_generator::jit_engine;
using sasl::code_generator::cgllvm_jit_engine;
using sasl::semantic::module_si;
using sasl::semantic::analysis_semantic;
using sasl::semantic::abi_analyser;
using sasl::syntax_tree::node;
using sasl::common::diag_chat;
using sasl::common::code_source;

using boost::shared_polymorphic_cast;
using boost::shared_ptr;

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::ostream;
using std::ofstream;

BEGIN_NS_SASL_DRIVER();

template <typename ParserT>
bool driver_impl::parse( ParserT& parser )
{
	try{
		opt_disp.reg_extra_parser( parser );
		opt_global.reg_extra_parser( parser );
		opt_io.reg_extra_parser( parser );
		opt_predef.reg_extra_parser( parser );

		po::parsed_options parsed
			= parser.run();

		std::vector<std::string> unrecg = po::collect_unrecognized( parsed.options, po::include_positional );

		if( !unrecg.empty() ){
			cout << "Warning: options ";
			BOOST_FOREACH( std::string const & str, unrecg ){
				cout << str << " ";
			}
			cout << "are invalid. They were ignored." << endl;
		}

		po::store( parsed, vm );
		po::notify(vm);

	} catch ( boost::program_options::invalid_command_line_syntax const & e ) {
		cout << "Fatal error occurs: " << e.what() << endl;
		return false;
	} catch ( std::exception const & e ){
		cout << "Unprocessed error: " << e.what() << endl;
	}

	return true;
}

void driver_impl::set_parameter( int argc, char** argv )
{
	po::basic_command_line_parser<char> parser
		= po::command_line_parser(argc, argv).options( desc ).allow_unregistered();
	if( !parse(parser) )
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}

void driver_impl::set_parameter( std::string const& cmd )
{
	vector<string> cmds = po::split_unix(cmd);
	po::basic_command_line_parser<char> parser
		= po::command_line_parser(cmds).options( desc ).allow_unregistered();

	if( !parse( parser ) ){
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}

driver_impl::driver_impl()
{
	opt_disp.fill_desc(desc);
	opt_global.fill_desc(desc);
	opt_io.fill_desc(desc);
	opt_predef.fill_desc(desc);
}

void driver_impl::compile()
{
	opt_disp.filterate(vm);
	opt_global.filterate(vm);
	opt_io.filterate(vm);
	opt_predef.filterate(vm);

	if( opt_disp.show_help ){
		cout << desc << endl;
		return;
	}

	if( opt_disp.show_version ){
		cout << opt_disp.version_info << endl;
		return;
	}

	if( opt_global.detail == options_global::none ){
		cout << "Detail level is an invalid value. Ignore it." << endl;
	}

	// Process inputs and outputs.
	vector<string> inputs = opt_io.in_names;

	if( inputs.empty() ){
		cout << "Need at least one input file." << endl;
		return;
	}

	// TODO
	salviar::languages lang = opt_io.lang;

	EFLIB_ASSERT_AND_IF( lang != salviar::lang_none, "Can not support language guessing by file extension yet." ){
		return;
	}

	if( opt_io.fmt == options_io::llvm_ir ){
		BOOST_FOREACH( string const & fname, inputs ){
			cout << "Compile " << fname << "..." << endl;

			shared_ptr<diag_chat> diags = diag_chat::create();

			shared_ptr<driver_code_source> code_src( new driver_code_source() );
			if ( !code_src->process_file(fname, diags.get()) ){
				diags->report( sasl::parser::cannot_open_input_file ) % fname;
				return;
			} 

			mroot = sasl::syntax_tree::parse( code_src.get(), code_src, diags.get() );
			if( !mroot ){
				cout << "Syntax error occurs!" << endl;
				return;
			}

			msi = analysis_semantic( mroot.get() );
			if( !msi ){
				cout << "Semantic error occurs!" << endl;
				return;
			}

			abi_analyser aa;

			if( !aa.auto_entry( msi, lang ) ){
				if ( lang != salviar::lang_general ){
					cout << "ABI analysis error occurs!" << endl;
					return;
				}
			}

			shared_ptr<llvm_module> llvmcode = generate_llvm_code( msi.get(), aa.abii(lang) );
			mcg = llvmcode;

			if( !llvmcode ){
				cout << "Code generation error occurs!" << endl;
				return;
			}

			if( !opt_io.output_name.empty() ){
				ofstream out_file( opt_io.output_name.c_str(), std::ios_base::out );
				llvmcode->dump( out_file );
			}
		}
	}
}

shared_ptr< module_si > driver_impl::mod_si() const{
	return msi;
}

shared_ptr<codegen_context> driver_impl::mod_codegen() const{
	return mcg;
}

shared_ptr<node> driver_impl::root() const{
	return mroot;
}

po::variables_map const & driver_impl::variables() const
{
	return vm;
}

options_display_info const & driver_impl::display_info() const
{
	return opt_disp;
}

options_io const & driver_impl::io_info() const
{
	return opt_io;
}

void driver_impl::set_code_source( std::string const& )
{

}

void driver_impl::set_code_source( shared_ptr<code_source> const& )
{

}

void driver_impl::set_diag_chat( diag_chat* diags )
{

}

shared_ptr<jit_engine> driver_impl::create_jit()
{
	std::string err;
	return cgllvm_jit_engine::create( shared_polymorphic_cast<llvm_module>(mcg), err );
}

END_NS_SASL_DRIVER();