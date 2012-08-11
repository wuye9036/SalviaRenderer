#include <sasl/include/driver/driver_impl.h>

#include <sasl/include/driver/driver_diags.h>
#include <sasl/include/driver/code_sources.h>
#include <sasl/include/driver/options.h>

#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>
#include <sasl/include/semantic/semantic_api.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantics.h>
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

using sasl::code_generator::cgllvm_module;
using sasl::code_generator::generate_llvm_code;
using sasl::code_generator::cgllvm_module;
using sasl::code_generator::jit_engine;
using sasl::code_generator::cgllvm_jit_engine;
using sasl::semantic::module_semantic;
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
		opt_macros.reg_extra_parser( parser );
		opt_includes.reg_extra_parser(parser);

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
#if defined(EFLIB_WINDOWS)
	vector<string> cmds = po::split_winmain(cmd);
#else
	vector<string> cmds = po::split_unix(cmd);
#endif

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
	opt_macros.fill_desc(desc);
	opt_includes.fill_desc(desc);
}

shared_ptr<diag_chat> driver_impl::compile()
{
	// Initialize env for compiling. 
	shared_ptr<diag_chat> diags = diag_chat::create();

	opt_disp.filterate(vm);
	opt_global.filterate(vm);
	opt_io.filterate(vm);
	opt_macros.filterate(vm);
	opt_includes.filterate(vm);

	if( opt_disp.show_help ){
		diags->report(text_only)->p(desc);
		return diags;
	}

	if( opt_disp.show_version ){
		diags->report(text_only)->p(opt_disp.version_info);
		return diags;
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
			return diags;
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
		return diags;
	}

	// Set include and virtual include.

	driver_code_source* driver_sc = dynamic_cast<driver_code_source*>( code_src.get() );

	if( driver_sc )
	{
		driver_sc->set_diag_chat( diags.get() );

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

			// Apply macros from API.
			for(size_t i = 0; i < macros.size(); ++i){
				macro_states ms = macros[i].second;
				switch (ms)
				{
				case ms_normal:
					driver_sc->add_macro( macros[i].first, false );
					break;
				case ms_predef:
					driver_sc->add_macro( macros[i].first, true );
					break;
				case ms_remove:
					driver_sc->remove_macro( macros[i].first );
					break;
				}
			}

			// Apply macros from parameters.
			driver_sc->add_macro( opt_macros.defines, false );
			driver_sc->add_macro( opt_macros.predefs, true );
			driver_sc->remove_macro( opt_macros.undefs );

			// Apply search path from API
			driver_sc->add_include_path( inc_paths );
			driver_sc->add_sys_include_path( sys_paths );

			// Apply search path from parameters.
			driver_sc->add_include_path( opt_includes.includes );
			driver_sc->add_sys_include_path( opt_includes.sysincls );
		}
	}

	// Compiling
	mroot = sasl::syntax_tree::parse( code_src.get(), lex_ctxt, diags.get() );
	if( !mroot ){ return diags; }

	shared_ptr<diag_chat> semantic_diags = diag_chat::create();
	msem = analysis_semantic( mroot.get(), semantic_diags.get(), lang );
	if( error_count( semantic_diags.get(), false ) > 0 )
	{
		msem.reset();
	}
	diag_chat::merge( diags.get(), semantic_diags.get(), true );

	if( !msem ){ return diags; }

	abi_analyser aa;

	if( !aa.auto_entry(msem, lang) ){
		if ( lang != salviar::lang_general ){
			cout << "ABI analysis error occurs!" << endl;
			return diags;
		}
	}
	mabi = aa.shared_abii(lang);

	shared_ptr<cgllvm_module> llvmcode = generate_llvm_code( msem, mabi.get() );
	mod = llvmcode;

	if( !llvmcode ){
		cout << "Code generation error occurs!" << endl;
		return diags;
	}

	if( opt_io.fmt == options_io::llvm_ir ){
		if( !opt_io.output_file_name.empty() ){
			ofstream out_file( opt_io.output_file_name.c_str(), std::ios_base::out );
			llvmcode->dump_ir( out_file );
		}
	}
	return diags;
}

shared_ptr<module_semantic> driver_impl::module_sem() const{
	return msem;
}

shared_ptr<cgllvm_module> driver_impl::module() const{
	return mod;
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

// WORDAROUNDS_TODO LLVM 3.0 Intrinsic to native call error.
void sasl_exp_f32	( float* ret, float v ) { *ret = expf(v); }
void sasl_exp2_f32	( float* ret, float v ) { *ret = ldexpf(1.0f, v); }
void sasl_sin_f32	( float* ret, float v ) { *ret = sinf(v); }
void sasl_cos_f32	( float* ret, float v ) { *ret = cosf(v); }
void sasl_tan_f32	( float* ret, float v ) { *ret = tanf(v); }
void sasl_sinh_f32	( float* ret, float v ) { *ret = sinhf(v); }
void sasl_cosh_f32	( float* ret, float v ) { *ret = coshf(v); }
void sasl_tanh_f32	( float* ret, float v ) { *ret = tanhf(v); }
void sasl_asin_f32	( float* ret, float v ) { *ret = asinf(v); }
void sasl_acos_f32	( float* ret, float v ) { *ret = acosf(v); }
void sasl_atan_f32	( float* ret, float v ) { *ret = atanf(v); }
void sasl_ceil_f32	( float* ret, float v ) { *ret = eflib::fast_ceil(v); }
void sasl_floor_f32	( float* ret, float v ) { *ret = eflib::fast_floor(v); }
void sasl_round_f32	( float* ret, float v ) { *ret = eflib::fast_round(v); }
void sasl_log_f32	( float* ret, float v ) { *ret = eflib::fast_log(v); }
void sasl_log10_f32	( float* ret, float v ) { *ret = log10f(v); }
void sasl_log2_f32	( float* ret, float v ) { *ret = eflib::fast_log2(v); }
void sasl_rsqrt_f32	( float* ret, float v ) { *ret = 1.0f / sqrtf(v); }
void sasl_mod_f32	( float* ret, float lhs, float rhs ){ *ret = fmodf(lhs, rhs); }
void sasl_ldexp_f32	( float* ret, float lhs, float rhs ){ *ret = ldexpf(lhs, rhs); }
void sasl_pow_f32	( float* ret, float lhs, float rhs ){ *ret = powf(lhs, rhs); }
void sasl_countbits_u32(uint32_t* ret, uint32_t v)
{
	*ret = 0;
	while(v) {
		++(*ret);
		v &= (v-1);
	}
}

void sasl_firstbithigh_u32(uint32_t* ret, uint32_t v)
{
	unsigned long index;
	_BitScanReverse(&index, v);
	*ret = static_cast<uint32_t>(31-index);
}

void sasl_firstbitlow_u32(uint32_t* ret, uint32_t v)
{
	unsigned long index;
	_BitScanForward(&index, v);
	*ret = static_cast<uint32_t>(index);
}

shared_ptr<jit_engine> driver_impl::create_jit()
{
	std::string err;
	if(!mod){
		return shared_ptr<jit_engine>();
	}
	shared_ptr<cgllvm_jit_engine> ret_jit = cgllvm_jit_engine::create( shared_polymorphic_cast<cgllvm_module>(mod), err );

	// WORKAROUND_TODO LLVM 3.0 Some intrinsic generated incorrect function call.
	inject_function(ret_jit, &sasl_exp_f32,		"sasl.exp.f32",		true);
	inject_function(ret_jit, &sasl_mod_f32,		"sasl.mod.f32",		true);
	inject_function(ret_jit, &sasl_exp2_f32,	"sasl.exp2.f32",	true);
	inject_function(ret_jit, &sasl_sin_f32,		"sasl.sin.f32",		true);
	inject_function(ret_jit, &sasl_cos_f32,		"sasl.cos.f32",		true);
	inject_function(ret_jit, &sasl_tan_f32,		"sasl.tan.f32",		true);
	inject_function(ret_jit, &sasl_asin_f32,	"sasl.asin.f32",	true);
	inject_function(ret_jit, &sasl_acos_f32,	"sasl.acos.f32",	true);
	inject_function(ret_jit, &sasl_atan_f32,	"sasl.atan.f32",	true);
	inject_function(ret_jit, &sasl_ceil_f32,	"sasl.ceil.f32",	true);
	inject_function(ret_jit, &sasl_floor_f32,	"sasl.floor.f32",	true);
	inject_function(ret_jit, &sasl_round_f32,	"sasl.round.f32",	true);
	inject_function(ret_jit, &sasl_log_f32,		"sasl.log.f32",		true);
	inject_function(ret_jit, &sasl_log2_f32,	"sasl.log2.f32",	true);
	inject_function(ret_jit, &sasl_log10_f32,	"sasl.log10.f32",	true);
	inject_function(ret_jit, &sasl_rsqrt_f32,	"sasl.rsqrt.f32",	true);
	inject_function(ret_jit, &sasl_ldexp_f32,	"sasl.ldexp.f32",	true);
	inject_function(ret_jit, &sasl_pow_f32,		"sasl.pow.f32",		true);
	inject_function(ret_jit, &sasl_sinh_f32,	"sasl.sinh.f32",	true);
	inject_function(ret_jit, &sasl_cosh_f32,	"sasl.cosh.f32",	true);
	inject_function(ret_jit, &sasl_tanh_f32,	"sasl.tanh.f32",	true);

	inject_function(ret_jit, &sasl_countbits_u32, "sasl.countbits.u32", true);
	inject_function(ret_jit, &sasl_firstbithigh_u32, "sasl.firstbithigh.u32", true);
	inject_function(ret_jit, &sasl_firstbitlow_u32 , "sasl.firstbitlow.u32" , true);

	return ret_jit;
}

shared_ptr<jit_engine> driver_impl::create_jit( external_function_array const& extfns )
{
	shared_ptr<jit_engine> ret_jit = create_jit();
	if(ret_jit)
	{
		for( size_t i = 0; i < extfns.size(); ++i )
		{
			inject_function( ret_jit, extfns[i].get<0>(), extfns[i].get<1>(), extfns[i].get<2>() );
		}
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
		raw_name = &( msem->root_symbol()->find_overloads(name)[0]->mangled_name() );
	}
	
	je->inject_function(pfn, *raw_name);
}

void driver_impl::add_sysinclude_path( std::string const& sys_path )
{
	sys_paths.push_back(sys_path);
}

void driver_impl::add_include_path( std::string const& inc_path )
{
	inc_paths.push_back(inc_path);
}

void driver_impl::clear_sysinclude_paths()
{
	sys_paths.clear();
}

void driver_impl::add_macro( std::string const& macro, bool predef )
{
	macros.push_back( make_pair(macro, predef?ms_predef:ms_normal) );
}

void driver_impl::clear_macros()
{
	macros.clear();
}

void driver_impl::remove_macro( std::string const& macro )
{
	macros.push_back( make_pair(macro, ms_remove) );
}


void driver_null::set_parameter( int argc, char** argv )
{

}

driver_null::driver_null()
{

}

END_NS_SASL_DRIVER();