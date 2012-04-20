#include <sasl/include/driver/driver_impl.h>

#include <sasl/include/driver/driver_diags.h>
#include <sasl/include/driver/code_sources.h>
#include <sasl/include/driver/options.h>

#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>
#include <sasl/include/semantic/semantic_api.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>
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
using sasl::semantic::abi_info;
using sasl::syntax_tree::node;
using sasl::common::lex_context;
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
	: user_diags(NULL)
{
	opt_disp.fill_desc(desc);
	opt_global.fill_desc(desc);
	opt_io.fill_desc(desc);
	opt_predef.fill_desc(desc);
}

void driver_impl::compile()
{
	// Initialize env for compiling.
	shared_ptr<diag_chat> local_diags;
	diag_chat* diags = NULL;
	if( user_diags )
	{
		diags = user_diags;
	}
	else 
	{
		local_diags = diag_chat::create();
		diags = local_diags.get();
	}

	opt_disp.filterate(vm);
	opt_global.filterate(vm);
	opt_io.filterate(vm);
	opt_predef.filterate(vm);

	if( opt_disp.show_help ){
		diags->report(text_only)->p(desc);
		return;
	}

	if( opt_disp.show_version ){
		diags->report(text_only)->p(opt_disp.version_info);
		return;
	}

	if( opt_global.detail == options_global::none ){
		diags->report(unknown_detail_level)->p(opt_global.detail_str);
	}

	salviar::languages lang = opt_io.lang;
	if( lang == salviar::lang_none ) {
		diags->report(unknown_lang);
	}

	// Process inputs and outputs.
	std::string fname = opt_io.input_file;
	shared_ptr<code_source> code_src;
	shared_ptr<lex_context> lex_ctxt;
	// Set code source.
	if( !fname.empty() )
	{
		shared_ptr<driver_code_source> file_code_source( new driver_code_source() );
		
		if ( !file_code_source->set_file(fname) ){
			diags->report( sasl::parser::cannot_open_input_file )->p(fname);
			return;
		} 

		diags->report(compiling_input)->p(fname);
		code_src = file_code_source;
		lex_ctxt = file_code_source;
	}
	else if( user_code_src )
	{
		code_src = user_code_src;
		lex_ctxt = user_lex_ctxt;
	}
	else
	{
		diags->report(input_file_is_missing);
		return;
	}

	// Set include and virtual include.

	driver_code_source* driver_sc = dynamic_cast<driver_code_source*>( code_src.get() );

	if( driver_sc )
	{
		driver_sc->set_diag_chat( diags );

		if( user_inc_handler )
		{
			driver_sc->set_include_handler( user_inc_handler );
		}
		else
		{
			for( virtual_file_dict::iterator it = virtual_files.begin(); it != virtual_files.end(); ++it)
			{
				driver_sc->add_virtual_file( it->first, it->second.first, it->second.second );
			}
		}
	}

	// Compiling
	mroot = sasl::syntax_tree::parse( code_src.get(), lex_ctxt, diags );
	if( !mroot ){ return; }

	shared_ptr<diag_chat> semantic_diags = diag_chat::create();
	msi = analysis_semantic( mroot.get(), semantic_diags.get() );
	if( error_count( semantic_diags.get(), false ) > 0 )
	{
		msi.reset();
	}
	diag_chat::merge( diags, semantic_diags.get(), true );

	if( !msi ){ return; }

	abi_analyser aa;

	if( !aa.auto_entry(msi, lang) ){
		if ( lang != salviar::lang_general ){
			cout << "ABI analysis error occurs!" << endl;
			return;
		}
	}
	mabi = aa.shared_abii(lang);

	shared_ptr<llvm_module> llvmcode = generate_llvm_code( msi.get(), mabi.get() );
	mcg = llvmcode;

	if( !llvmcode ){
		cout << "Code generation error occurs!" << endl;
		return;
	}

	if( opt_io.fmt == options_io::llvm_ir ){
		if( !opt_io.output_file_name.empty() ){
			ofstream out_file( opt_io.output_file_name.c_str(), std::ios_base::out );
			llvmcode->dump( out_file );
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

void driver_impl::set_code( std::string const& code )
{
	shared_ptr<driver_code_source> src( new driver_code_source() );
	src->set_code( code );
	user_code_src = src;
	user_lex_ctxt = src;
}

void driver_impl::set_code_source( shared_ptr<code_source> const& src )
{
	user_code_src = src;
}

void driver_impl::set_diag_chat( diag_chat* diags )
{
	user_diags = diags;
}

// WORDAROUNDS_TODO LLVM 3.0 Intrinsic to native call error.
void workaround_expf( float* ret, float v )
{
	*ret = expf(v);
}

void workaround_fmodf( float* ret, float lhs, float rhs )
{
	*ret = fmodf(lhs, rhs);
}

shared_ptr<jit_engine> driver_impl::create_jit()
{
	std::string err;
	shared_ptr<cgllvm_jit_engine> ret_jit = cgllvm_jit_engine::create( shared_polymorphic_cast<llvm_module>(mcg), err );

	// WORKAROUND_TODO LLVM 3.0 Some intrinsic generated incorrect function call.
	inject_function(ret_jit, &workaround_expf, "__wa_expf", false);
	inject_function(ret_jit, &workaround_fmodf, "__wa_fmodf", false);
	return ret_jit;
}

shared_ptr<jit_engine> driver_impl::create_jit( external_function_array const& extfns )
{
	shared_ptr<jit_engine> ret_jit = create_jit();
	for( size_t i = 0; i < extfns.size(); ++i )
	{
		inject_function( ret_jit, extfns[i].get<0>(), extfns[i].get<1>(), extfns[i].get<2>() );
	}
	return ret_jit;
}

void driver_impl::set_code_file( std::string const& code_file )
{
	opt_io.input_file = code_file;
}

void driver_impl::set_lex_context( shared_ptr<lex_context> const& lex_ctxt )
{
	user_lex_ctxt = lex_ctxt;
}

void driver_impl::add_virtual_file( string const& file_name, string const& code_content, bool high_priority )
{
	virtual_files[file_name] = make_pair( code_content, high_priority );
}

void driver_impl::set_include_handler( include_handler_fn inc_handler )
{
	user_inc_handler = inc_handler;
}

shared_ptr<abi_info> driver_impl::mod_abi() const
{
	return mabi;
}

void driver_impl::inject_function(shared_ptr<jit_engine> const& je, void* pfn, string const& name, bool is_raw_name )
{
	std::string const* raw_name;
	if( is_raw_name )
	{
		raw_name = &name;
	}
	else
	{
		raw_name = &( msi->root()->find_overloads(name)[0]->mangled_name() );
	}
	
	je->inject_function(pfn, *raw_name);
}

END_NS_SASL_DRIVER();