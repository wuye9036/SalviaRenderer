#include <sasl/include/compiler/options.h>

#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/common/lex_context.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/semantic_api.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/parse_api.h>
#include <sasl/include/syntax_tree/program.h>

#include <softart/include/enums.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/platform/boost_begin.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <eflib/include/platform/boost_end.h>

#include <boost/exception/all.hpp>
#include <iostream>
#include <fstream>

using sasl::code_generator::codegen_context;
using sasl::code_generator::llvm_module;
using sasl::code_generator::generate_llvm_code;
using sasl::common::lex_context;
using sasl::semantic::abi_analyser;
using sasl::semantic::module_si;
using sasl::semantic::analysis_semantic;
using sasl::syntax_tree::node;
using sasl::syntax_tree::parse;
using sasl::syntax_tree::program;

using boost::make_shared;
using boost::scoped_ptr;
using boost::shared_polymorphic_cast;
using boost::shared_ptr;
using boost::to_lower;

using std::cout;
using std::endl;
using std::ifstream;
using std::locale;
using std::isalnum;
using std::isalpha;
using std::make_pair;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

class compiler_code_source: public lex_context, public sasl::common::code_source{

private:
	typedef boost::wave::cpplexer::lex_iterator<
		boost::wave::cpplexer::lex_token<> >
		wlex_iterator_t;
	typedef boost::wave::context<
		string::iterator, wlex_iterator_t>
		wcontext_t;

public:
	bool process( std::string const& file_name ){
		std::ifstream in(file_name.c_str(), std::ios_base::in);
		if (!in){
			return false;
		} else {
			in.unsetf(std::ios::skipws);
			std::copy(
				std::istream_iterator<char>(in), std::istream_iterator<char>(),
				std::back_inserter(code) );
			wctxt.reset( new wcontext_t( code.begin(), code.end() ) );

			size_t lang_flag = wctxt->get_language();
			lang_flag &= ~(boost::wave::support_option_emit_line_directives );
			lang_flag &= ~(boost::wave::support_option_single_line );
			lang_flag &= ~(boost::wave::support_option_emit_pragma_directives );
			wctxt->set_language( static_cast<boost::wave::language_support>( lang_flag ) );

			cur_it = wctxt->begin();
			next_it = wctxt->begin();
		}
		in.close();

		return true;
	}

	// code source
	virtual bool is_eof(){
		return next_it == wctxt->end();
	}

	virtual std::string next_token(){
		assert( next_it != wctxt->end() );
		cur_it = next_it;

		try{
			++next_it;
		} catch ( boost::wave::preprocess_exception& e ){
			next_it = wctxt->end();
			cout << e.description() << endl;
		}

		return std::string( (*cur_it).get_value().begin(), (*cur_it).get_value().end() ) ;
	}

	// lex_context
	virtual const std::string& file_name() const{
		assert( cur_it != wctxt->end() );

		filename = to_std_string( cur_it->get_position().get_file() );
		return filename;
	}
	virtual size_t column() const{
		assert( cur_it != wctxt->end() );
		return cur_it->get_position().get_column();
	}
	virtual size_t line() const{
		assert( cur_it != wctxt->end() );
		return cur_it->get_position().get_line();
	}

	virtual void next( const std::string& /*lit*/ ){
		// Do nothing.
		return;
	}
private:
	template<typename StringT>
	std::string to_std_string( StringT const& str ) const{
		return std::string( str.begin(), str.end() );
	}

	scoped_ptr<wcontext_t> wctxt;

	std::string code;
	mutable std::string filename;

	wcontext_t::iterator_type cur_it;
	wcontext_t::iterator_type next_it;
};

BEGIN_NS_SASL_COMPILER();

options_manager options_manager::inst;

options_manager& options_manager::instance(){
	return inst;
}

bool options_manager::parse( int argc, char** argv )
{
	try{

		po::basic_command_line_parser<char> parser
			= po::command_line_parser(argc, argv).options( desc ).allow_unregistered();

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

options_manager::options_manager()
{
	opt_disp.fill_desc(desc);
	opt_global.fill_desc(desc);
	opt_io.fill_desc(desc);
	opt_predef.fill_desc(desc);
}

void options_manager::process( bool& abort )
{
	abort = false;

	opt_disp.filterate(vm);
	opt_global.filterate(vm);
	opt_io.filterate(vm);
	opt_predef.filterate(vm);

	opt_disp.process(abort);
	if( abort ){ return; }

	opt_global.process(abort);
	if( abort ){ return; }

	opt_io.process(abort);
	if( abort ){ return; }

	opt_predef.process(abort);
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

// options filter
void options_filter::reg_extra_parser( po::basic_command_line_parser<char>& ){
}

//////////////////////////////////////////////////////////////////////////
// display info

const char* options_display_info::version_tag = "version,v";
const char* options_display_info::version_desc = "Show version and copyright information";
const char* options_display_info::version_info = 
	"SoftArt/SALVIA Shading Language Compiler(sac) 1.0 pre-alpha\r\n"
	"Copyright (C) 2010 SoftArt/SALVIA Development Group."
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

const char* options_io::lang_tag = "lang";
const char* options_io::lang_desc = "Specifies language the input file was treated as.'general(g)','vertex_shader(vs)','pixel_shader(ps)','blend_shader(bs)' are available. ";

options_io::options_io() : fmt(none), lang(softart::lang_none)
{
}

void options_io::fill_desc( po::options_description& desc )
{
	desc.add_options()
		( in_tag, po::value< std::vector<string> >(&in_names), in_desc )
		( out_tag, po::value< string >(&out_name), out_desc )
		( export_as_tag, po::value< string >(&fmt_str), export_as_desc )
		( lang_tag, po::value< string >(&lang_str), lang_desc )
		;
}

void options_io::filterate( po::variables_map const & vm )
{
	if( !vm.count("out") ){
		// TODO Guess output from input.
	}

	if( !vm.count("export-as") ){
		fmt = llvm_ir;
	} else {
		to_lower(fmt_str);
		if( fmt_str == "llvm" || fmt_str == "llvm_ir" ){
			fmt = llvm_ir;
		}
	}

	if ( !vm.count("lang") ){
		lang = softart::lang_none;
	} else {
		to_lower( lang_str );
		if ( lang_str == "general" || lang_str == "g" ){
			lang = softart::lang_general;
		} else if ( lang_str == "vertex_shader" || lang_str == "vs" ){
			lang = softart::lang_vertex_sl;
		} else if ( lang_str == "pixel_shader" || lang_str == "ps" ){
			lang = softart::lang_pixel_sl;
		} else if ( lang_str == "blend_shader" || lang_str == "bs" ){
			lang = softart::lang_blend_sl;
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

	// TODO
	EFLIB_ASSERT_AND_IF( lang != softart::lang_none, "Can not support language guessing by file extension yet." ){
		abort = true;
		return;
	}

	if( fmt == llvm_ir ){
		BOOST_FOREACH( string const & fname, in_names ){
			cout << "Compile " << fname << "..." << endl;
			
			shared_ptr<compiler_code_source> code_src( new compiler_code_source() );
			if ( !code_src->process(fname) ){
				cout << "Fatal error: Could not open input file: " << fname << endl;
			} else {
				shared_ptr<node> mroot = parse( code_src.get(), code_src );
				if( !mroot ){
					cout << "Syntax error occurs!" << endl;
					abort = true;
					return;
				}

				msi = analysis_semantic( mroot );
				if( !msi ){
					cout << "Semantic error occurs!" << endl;
					abort = true;
					return;
				}
				
				abi_analyser aa;

				if( !aa.auto_entry( msi, lang ) ){
					if ( lang != softart::lang_general ){
						cout << "ABI analysis error occurs!" << endl;
						abort = true;
						return;
					}
				}

				shared_ptr<llvm_module> llvmcode = generate_llvm_code( msi.get(), aa.abii(lang) );
				mcg = llvmcode;

				if( !llvmcode ){
					cout << "Code generation error occurs!" << endl;
					abort = true;
					return;
				}

				if( !output().empty() ){
					ofstream out_file( output().c_str(), std::ios_base::out );
					dump( llvmcode, out_file );
				}
			}
			
		}
	}
}

softart::languages options_io::language() const{
	return lang;
}

options_io::export_format options_io::format() const
{
	return fmt;
}

std::string options_io::output() const
{
	return out_name;
}

shared_ptr< module_si > options_io::module_sem() const{
	return msi;
}

shared_ptr<codegen_context> options_io::module_codegen() const{
	return mcg;
}

shared_ptr<node> options_io::root() const{
	return mroot;
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

// options_predefinition

const char* options_predefinition::define_tag = "define,D";
const char* options_predefinition::define_desc = "-D<name><=><text>, Define macros.";

options_predefinition::options_predefinition(){
	return;
}

void options_predefinition::reg_extra_parser( po::basic_command_line_parser<char>& cmdpar ){
	cmdpar.extra_parser( boost::bind( &options_predefinition::parse_predef, this, _1 ) );
}

void options_predefinition::fill_desc( po::options_description& desc ){
	desc.add_options()
		( define_tag, define_desc )
		;
}

void options_predefinition::filterate( po::variables_map const & vm ){
	// Do nothing. parse_predef will hook all legal definitions.
	return;
}

void options_predefinition::process( bool& abort ){
	return;
}

pair<string, string> options_predefinition::parse_predef( string const& str )
{
	pair<string, string> null_ret( string(""), string("") );

	if( str.find("-D") != 0 ){ return null_ret; }
	if( str.length() <= 2 ){ return null_ret; }

	// Split cmd
	string::const_iterator equal_it = find(str.begin(), str.end(), '=');

	string::const_iterator beg_it = str.begin()+2;
	if( isalpha( *beg_it, locale() ) || *beg_it == '_' ){
		// Check define name
		for( string::const_iterator it = beg_it; it != equal_it; ++it ){
			if( !isalnum(*beg_it) && *beg_it != '_' ){
				return null_ret;
			}
		}
		string name( beg_it, equal_it );

		// Split define content
		string content;
		if( equal_it != str.end() ){
			string::const_iterator content_it = equal_it + 1;
			if( 
				*content_it == '"'
				&& content_it+1 != str.end()
				&& *str.rbegin() == '"'
				)
			{
				content.assign( content_it+1, str.end()-1 );
			} else {
				content.assign( content_it, str.end() );
			}
		}

		defs.push_back( make_pair(name, content) );
		return defs.back();

	} else {
		return null_ret;
	}
}
END_NS_SASL_COMPILER();