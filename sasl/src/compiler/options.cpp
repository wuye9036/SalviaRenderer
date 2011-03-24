#include <sasl/include/compiler/options.h>

#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/common/lex_context.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/semantic_api.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/parse_api.h>
#include <sasl/include/syntax_tree/program.h>

#include <softart/include/enums.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

#include <iostream>
#include <fstream>

using sasl::code_generator::llvm_code;
using sasl::code_generator::generate_llvm_code;
using sasl::common::lex_context;
using sasl::semantic::abi_analyser;
using sasl::semantic::module_si;
using sasl::semantic::analysis_semantic;
using sasl::syntax_tree::node;
using sasl::syntax_tree::parse;
using sasl::syntax_tree::program;

using boost::make_shared;
using boost::shared_polymorphic_cast;
using boost::shared_ptr;
using boost::to_lower;

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

class lex_context_test_impl: public lex_context{
public:
	virtual const std::string& file_name() const{
		return filename;
	}
	virtual size_t column() const{
		return 0;
	}
	virtual size_t line() const{
		return 0;
	}

	virtual void next( const std::string& /*lit*/ ){
		return;
	}

private:
	std::string filename;
};

BEGIN_NS_SASL_COMPILER();

options_manager options_manager::inst;

options_manager& options_manager::instance()
{
	return inst;
}

bool options_manager::parse( int argc, char** argv )
{
	try{

		po::parsed_options parsed = po::command_line_parser(argc, argv).options( desc ).allow_unregistered().run();

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

options_manager::options_manager()
{
	opt_disp.fill_desc(desc);
	opt_global.fill_desc(desc);
	opt_io.fill_desc(desc);
}

void options_manager::process( bool& abort )
{
	abort = false;

	opt_disp.filterate(vm);
	opt_global.filterate(vm);
	opt_io.filterate(vm);

	opt_disp.process(abort);
	if( abort ){ return; }

	opt_global.process(abort);
	if( abort ){ return; }

	opt_io.process(abort);
	if( abort ){ return; }
}

po::variables_map const & options_manager::variables() const
{
	return vm;
}

options_display_info const & options_manager::display_info() const
{
	return opt_disp;
}

options_io const & options_manager::io_info() const
{
	return opt_io;
}

//////////////////////////////////////////////////////////////////////////
// display info

const char* options_display_info::version_tag = "version,v";
const char* options_display_info::version_desc = "Show version and copyright information";
const char* options_display_info::version_info = 
	"SoftArt/Salvia Shading Language Compiler(sac) 1.0 pre-alpha\r\n"
	"Copyright (C) 2010 SoftArt/Salvia Development Group."
	"This software and its full source code copyright is GPLv2.";

const char* options_display_info::help_tag = "help,h";
const char* options_display_info::help_desc = "Display this information.";

options_display_info::options_display_info()
	: h(false), v(false)
{}

void options_display_info::fill_desc( po::options_description& desc )
{
	desc.add_options()
		( help_tag, help_desc)
		( version_tag, version_desc )
		;
	pdesc = &desc;
}

void options_display_info::filterate( po::variables_map const & vm )
{
	h = ( vm.empty() || vm.count("help") > 0 );
	v = ( vm.count("version") > 0 );
}

void options_display_info::process( bool& abort )
{
	if( h ){
		cout << *pdesc << endl;
		abort = true;
		return;
	}

	if( v ){
		cout << version_info << endl;
		abort = false;
	}
}

bool options_display_info::help_enabled() const
{
	return h;
}

bool options_display_info::version_enabled() const
{
	return v;
}

//////////////////////////////////////////////////////////////////////////
// input & output

const char* options_io::in_tag = "input,i";
const char* options_io::in_desc = "Specify input files.";

const char* options_io::out_tag = "out,o";
const char* options_io::out_desc = "File name of output.";

const char* options_io::export_as_tag = "export-as";
const char* options_io::export_as_desc = "Specifies the content of output file that the compiler should generate.";

options_io::options_io() : fmt(none)
{
}

void options_io::fill_desc( po::options_description& desc )
{
	desc.add_options()
		( in_tag, po::value< std::vector<string> >(&in_names), in_desc )
		( out_tag, po::value< string >(&out_name), out_desc )
		( export_as_tag, po::value< string >(&fmt_str), export_as_desc )
		;
}

void options_io::filterate( po::variables_map const & vm )
{
	if( !vm.count("out") ){
		// Guess output from input.
	}

	if( !vm.count("export-as") ){
		fmt = llvm_ir;
	} else {
		to_lower(fmt_str);
		if( fmt_str == "llvm" || fmt_str == "llvm_ir" ){
			fmt = llvm_ir;
		}
	}
}

void options_io::process( bool& abort )
{
	if( in_names.empty() ){
		cout << "No input files." << endl;
		abort = true;
		return;
	}
	if( fmt == llvm_ir ){
		BOOST_FOREACH( string const & fname, in_names ){
			cout << "Compile " << fname << "..." << endl;
			
			std::ifstream in(fname, std::ios_base::in);
			if (!in){
				cout << "Fatal error: Could not open input file: " << fname << endl;
			} else {
				string code;
				in.unsetf(std::ios::skipws);
				std::copy(
					std::istream_iterator<char>(in), std::istream_iterator<char>(),
					std::back_inserter(code)
					);
				
				shared_ptr<node> prog = parse( code, make_shared<lex_context_test_impl>() );
				if( !prog ){
					cout << "Syntax error happened!" << endl;
					return;
				}
				// TODO: For test only.
				shared_ptr<module_si> msi = analysis_semantic( prog );
				if( !msi ){
					cout << "Semantic error happened!" << endl;
				}
				
				abi_analyser aa;
				aa.auto_entry( msi, softart::lang_general );

				shared_ptr<llvm_code> llvmcode = generate_llvm_code( msi.get(), aa.abii(softart::lang_general) );
				if( !llvmcode ){
					cout << "Code generation error happened!" << endl;
				}

				if( !output().empty() ){
					ofstream out_file( output(), std::ios_base::out );
					dump( llvmcode, out_file );
				}
			}
			
		}
	}
}

options_io::export_format options_io::format() const
{
	return fmt;
}

std::string options_io::output() const
{
	return out_name;
}

//////////////////////////////////////////////////////////////////////////
// options global
void options_global::fill_desc( po::options_description& desc )
{
	desc.add_options()
		(
		"detail-level", po::value<string>(&detail_lvl_str),
		"Specify the detail level of compiler output."
		"The optional items are: quite(q), brief(b), normal(n), verbose(v)."
		"Default is normal"
		);
}

void options_global::filterate( po::variables_map const & vm )
{
	detail_lvl = normal;
	if( vm.count("detail-level") ){

		to_lower( detail_lvl_str );

		if( detail_lvl_str == "quite" || detail_lvl_str == "q" ){
			detail_lvl = quite;
		} else if( detail_lvl_str == "brief" || detail_lvl_str == "b" ){
			detail_lvl = brief;
		} else if( detail_lvl_str == "normal" || detail_lvl_str == "n" ){
			detail_lvl = normal;
		} else if( detail_lvl_str == "verbose" || detail_lvl_str == "v" ){
			detail_lvl = verbose;
		} else if( detail_lvl_str == "debug" || detail_lvl_str == "d" ){
			detail_lvl = debug;
		}

	}
}

void options_global::process( bool& abort )
{
	abort = false;
	if( detail_lvl == none ){
		cout << "Detail level is an invalid value. Ignore it." << endl;
	}
}

options_global::detail_level options_global::detail() const
{
	return detail_lvl;
}

END_NS_SASL_COMPILER();