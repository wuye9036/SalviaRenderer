#include <sasl/include/drivers/compiler_impl.h>

#include <sasl/include/drivers/compiler_diags.h>
#include <sasl/include/drivers/code_sources.h>
#include <sasl/include/drivers/options.h>

#include <sasl/include/codegen/cg_api.h>
#include <sasl/include/semantic/semantic_api.h>
#include <sasl/include/semantic/reflector.h>
#include <sasl/include/semantic/reflector2.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/parser/parse_api.h>
#include <sasl/include/parser/diags.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/common/diag_chat.h>

#include <salviar/include/shader_impl.h>

#include <eflib/include/diagnostics/profiler.h>
#include <eflib/include/string/ustring.h>
#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/platform/intrin.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <fstream>
#include <math.h>

namespace po = boost::program_options;

EFLIB_USING_SHARED_PTR(sasl::codegen, module_vmcode);
EFLIB_USING_SHARED_PTR(sasl::semantic, module_semantic);

EFLIB_USING_SHARED_PTR(sasl::semantic, reflection_impl);
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, node);
EFLIB_USING_SHARED_PTR(sasl::common, lex_context);
EFLIB_USING_SHARED_PTR(sasl::common, diag_chat);
EFLIB_USING_SHARED_PTR(sasl::common, code_source);

using sasl::semantic::analysis_semantic;
using sasl::semantic::reflect;
using sasl::codegen::generate_vmcode;

using salviar::external_function_desc;

using eflib::fixed_string;

using boost::dynamic_pointer_cast;
using boost::shared_ptr;

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::ostream;
using std::ofstream;

BEGIN_NS_SASL_DRIVERS();

template <typename ParserT>
bool compiler_impl::parse( ParserT& parser )
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
			for( std::string const & str: unrecg ){
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

void compiler_impl::set_parameter( int argc, char** argv )
{
	po::basic_command_line_parser<char> parser
		= po::command_line_parser(argc, argv).options( desc ).allow_unregistered();
	if( !parse(parser) )
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}

void compiler_impl::set_parameter( std::string const& cmd )
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

compiler_impl::compiler_impl()
{
	opt_disp.fill_desc(desc);
	opt_global.fill_desc(desc);
	opt_io.fill_desc(desc);
	opt_macros.fill_desc(desc);
	opt_includes.fill_desc(desc);
}

shared_ptr<diag_chat> compiler_impl::compile(bool enable_jit)
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
		shared_ptr<compiler_code_source> file_code_source( new compiler_code_source() );

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

	compiler_code_source* driver_sc = dynamic_cast<compiler_code_source*>( code_src.get() );

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

	eflib::profiler		prof;
	diag_chat_ptr		semantic_diags;

	{
		// Compiling with profiling

		eflib::profiling_scope prof_scope(&prof, "driver impl compiling");

		{
			eflib::profiling_scope prof_scope(&prof, "parse @ compiler_impl");
			mroot = sasl::syntax_tree::parse( code_src.get(), lex_ctxt, diags.get() );
			if( !mroot ){ return diags; }
		}

		{
			eflib::profiling_scope prof_scope(&prof, "semantic analysis @ compiler_impl");
			semantic_diags = diag_chat::create();
			msem = analysis_semantic( mroot.get(), semantic_diags.get(), lang );
			if( error_count( semantic_diags.get(), false ) > 0 )
			{
				msem.reset();
			}
			diag_chat::merge( diags.get(), semantic_diags.get(), true );
			if( !msem ){ return diags; }
		}

		{
			eflib::profiling_scope prof_scope(&prof, "ABI analysis @ compiler_impl");

			mreflection = reflect(msem, diags.get());
			mreflection2 = reflect2(msem);

			if(!mreflection)
			{
				if ( lang != salviar::lang_general )
				{
					cout << "ABI analysis error occurs!" << endl;
					return diags;
				}
			}
		}

		{
			eflib::profiling_scope prof_scope(&prof, "Code generation @ compiler_impl");

			mvmc = generate_vmcode( msem, mreflection.get() );
			if( !mvmc ){
				cout << "Code generation error occurs!" << endl;
				return diags;
			}

			if (enable_jit)
			{
				if( mvmc->enable_jit() )
				{
					inject_default_functions();
				}
			}
		}
	}

	// eflib::print_profiler(&prof, 3);

	if( opt_io.fmt == options_io::llvm_ir )
	{
		if( !opt_io.output_file_name.empty() )
		{
			ofstream out_file( opt_io.output_file_name.c_str(), std::ios_base::out );
			mvmc->dump_ir( out_file );
		}
	}
	return diags;
}

shared_ptr<diag_chat> compiler_impl::compile(vector<external_function_desc> const& external_funcs)
{
	shared_ptr<diag_chat> results = compile(true);
	if(!mvmc)
	{
		return results;
	}

	if( mvmc->enable_jit() )
	{
		for( size_t i = 0; i < external_funcs.size(); ++i )
		{
			inject_function(external_funcs[i].func, external_funcs[i].func_name, external_funcs[i].is_raw_name);
		}
	}

	return results;
}

module_semantic_ptr compiler_impl::get_semantic() const{
	return msem;
}

module_vmcode_ptr compiler_impl::get_vmcode() const{
	return mvmc;
}

node_ptr compiler_impl::get_root() const{
	return mroot;
}

po::variables_map const & compiler_impl::variables() const
{
	return vm;
}

options_display_info const & compiler_impl::display_info() const
{
	return opt_disp;
}

options_io const & compiler_impl::io_info() const
{
	return opt_io;
}

void compiler_impl::set_code( std::string const& code )
{
	shared_ptr<compiler_code_source> src( new compiler_code_source() );
	src->set_code( code );
	user_code_src = src;
	user_lex_ctxt = src;
}

void compiler_impl::set_code_source( shared_ptr<code_source> const& src )
{
	user_code_src = src;
}

// WORDAROUNDS_TODO LLVM 3.0 Intrinsic to native call error.
void sasl_exp_f32	( float* ret, float v ) { *ret = expf(v); }
void sasl_exp2_f32	( float* ret, float v ) { *ret = ldexpf(1.0f, static_cast<int>(v) ); }
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
void sasl_trunc_f32	( float* ret, float v ) { *ret = eflib::trunc(v); }
void sasl_log_f32	( float* ret, float v ) { *ret = eflib::fast_log(v); }
void sasl_log10_f32	( float* ret, float v ) { *ret = log10f(v); }
void sasl_log2_f32	( float* ret, float v ) { *ret = eflib::fast_log2(v); }
void sasl_rsqrt_f32	( float* ret, float v ) { *ret = 1.0f / sqrtf(v); }
void sasl_mod_f32	( float* ret, float lhs, float rhs ){ *ret = fmodf(lhs, rhs); }
void sasl_ldexp_f32	( float* ret, float lhs, float rhs ){ *ret = ldexpf(lhs, static_cast<int>(rhs) ); }
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
	uint32_t index;
	_xmm_bsr(&index, v);
	*ret = static_cast<uint32_t>(31-index);
}

void sasl_firstbitlow_u32(uint32_t* ret, uint32_t v)
{
	uint32_t index;
	_xmm_bsf(&index, v);
	*ret = static_cast<uint32_t>(index);
}

void sasl_reversebits_u32(uint32_t* ret, uint32_t v)
{
	v	= ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	v	= ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	v	= ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	v	= ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	*ret= ( v >> 16             ) | ( v               << 16);
}

void compiler_impl::inject_default_functions()
{
	if(!mvmc || !mvmc->enable_jit())
	{
		return;
	}

	// WORKAROUND_TODO LLVM 3.0 Some intrinsic generated incorrect function call.
	inject_function((void*)&sasl_exp_f32,	"sasl.exp.f32",		true);
	inject_function((void*)&sasl_mod_f32,	"sasl.mod.f32",		true);
	inject_function((void*)&sasl_exp2_f32,	"sasl.exp2.f32",	true);
	inject_function((void*)&sasl_sin_f32,	"sasl.sin.f32",		true);
	inject_function((void*)&sasl_cos_f32,	"sasl.cos.f32",		true);
	inject_function((void*)&sasl_tan_f32,	"sasl.tan.f32",		true);
	inject_function((void*)&sasl_asin_f32,	"sasl.asin.f32",	true);
	inject_function((void*)&sasl_acos_f32,	"sasl.acos.f32",	true);
	inject_function((void*)&sasl_atan_f32,	"sasl.atan.f32",	true);
	inject_function((void*)&sasl_ceil_f32,	"sasl.ceil.f32",	true);
	inject_function((void*)&sasl_floor_f32,	"sasl.floor.f32",	true);
	inject_function((void*)&sasl_round_f32,	"sasl.round.f32",	true);
	inject_function((void*)&sasl_trunc_f32,	"sasl.trunc.f32",	true);
	inject_function((void*)&sasl_log_f32,	"sasl.log.f32",		true);
	inject_function((void*)&sasl_log2_f32,	"sasl.log2.f32",	true);
	inject_function((void*)&sasl_log10_f32,	"sasl.log10.f32",	true);
	inject_function((void*)&sasl_rsqrt_f32,	"sasl.rsqrt.f32",	true);
	inject_function((void*)&sasl_ldexp_f32,	"sasl.ldexp.f32",	true);
	inject_function((void*)&sasl_pow_f32,	"sasl.pow.f32",		true);
	inject_function((void*)&sasl_sinh_f32,	"sasl.sinh.f32",	true);
	inject_function((void*)&sasl_cosh_f32,	"sasl.cosh.f32",	true);
	inject_function((void*)&sasl_tanh_f32,	"sasl.tanh.f32",	true);

	inject_function((void*)&sasl_countbits_u32,		"sasl.countbits.u32",	true);
	inject_function((void*)&sasl_firstbithigh_u32, 	"sasl.firstbithigh.u32",true);
	inject_function((void*)&sasl_firstbitlow_u32, 	"sasl.firstbitlow.u32" ,true);
	inject_function((void*)&sasl_reversebits_u32, 	"sasl.reversebits.u32" ,true);
}

void compiler_impl::set_code_file( std::string const& code_file )
{
	opt_io.input_file = code_file;
}

void compiler_impl::set_lex_context( shared_ptr<lex_context> const& lex_ctxt )
{
	user_lex_ctxt = lex_ctxt;
}

void compiler_impl::add_virtual_file( string const& file_name, string const& code_content, bool high_priority )
{
	virtual_files[file_name] = make_pair( code_content, high_priority );
}

void compiler_impl::set_include_handler( include_handler_fn inc_handler )
{
	user_inc_handler = inc_handler;
}

reflection_impl_ptr compiler_impl::get_reflection() const
{
	return mreflection;
}

salviar::shader_reflection2_ptr compiler_impl::get_reflection2() const
{
	return mreflection2;
}

void compiler_impl::inject_function(void* pfn, fixed_string const& fn_name, bool is_raw_name )
{
	eflib::fixed_string raw_name;
	if( is_raw_name )
	{
		raw_name = fn_name;
	}
	else
	{
		raw_name = msem->root_symbol()->find_overloads(fn_name)[0]->mangled_name();
	}

	mvmc->inject_function(pfn, raw_name);
}

void compiler_impl::add_sysinclude_path( std::string const& sys_path )
{
	sys_paths.push_back(sys_path);
}

void compiler_impl::add_include_path( std::string const& inc_path )
{
	inc_paths.push_back(inc_path);
}

void compiler_impl::clear_sysinclude_paths()
{
	sys_paths.clear();
}

void compiler_impl::add_macro( std::string const& macro, bool predef )
{
	macros.push_back( make_pair(macro, predef?ms_predef:ms_normal) );
}

void compiler_impl::clear_macros()
{
	macros.clear();
}

void compiler_impl::remove_macro( std::string const& macro )
{
	macros.push_back( make_pair(macro, ms_remove) );
}


void null_compiler::set_parameter( int /*argc*/, char** /*argv*/ )
{

}

null_compiler::null_compiler()
{

}

END_NS_SASL_DRIVERS();
