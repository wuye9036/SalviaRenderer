#include <sasl/include/host/host.h>

#include <sasl/include/common/lex_context.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/parse_api.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/abi_info.h>
#include <sasl/include/semantic/semantic_api.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/code_generator/jit_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>

#include <fstream>

using boost::shared_ptr;

using std::cout;
using std::endl;
using std::fstream;
using std::string;
using std::vector;

using boost::shared_static_cast;
using boost::shared_dynamic_cast;

using namespace salviar;
using namespace sasl::code_generator;
using namespace sasl::common;
using namespace sasl::syntax_tree;
using namespace sasl::host;
using namespace sasl::semantic;

BEGIN_NS_SASL_HOST();

shader_code_impl::shader_code_impl(): pfn(NULL){
}

void shader_code_impl::abii( shared_ptr<shader_abi> const& v ){
	abi = shared_dynamic_cast<abi_info>( v );
}

shader_abi const* shader_code_impl::abii() const{
	return abi.get();
}

void shader_code_impl::jit( shared_ptr<jit_engine> const& v ){
	je = v;
}

void* shader_code_impl::function_pointer() const{
	return pfn;
}

void shader_code_impl::update_native_function(){
	assert( abi && je );
	if( !abi || !je ){
		pfn = NULL;
		return;
	}
	pfn = je->get_function( abi->entry_name() );
}

void shader_code_impl::register_function( void* fnptr, std::string const& name )
{
	vector< shared_ptr<symbol> > syms = root_sym->find_overloads( name );
	if( syms.empty() ){ return; }
	je->inject_function( fnptr, syms[0]->mangled_name() );
}

void shader_code_impl::root( boost::shared_ptr<sasl::semantic::symbol> const& sym )
{
	root_sym = sym;
}

END_NS_SASL_HOST();

class shader_code_source: public lex_context, public code_source{
public:
	shader_code_source(): is_eof(true), fname("in_memory"){
	}

	bool process_code( std::string const& code ){
		this->code = code;
		return process();
	}

	// code source
	virtual bool eof(){ return is_eof; }
	virtual string error(){ return string(""); }
	virtual string next(){
		is_eof = true;
		return code;
	}

	// lex_context
	virtual const std::string& file_name() const{
		return fname;
	}
	virtual size_t column() const{
		return 0;
	}
	virtual size_t line() const{
		return 0;
	}

	virtual void update_position( const std::string& /*lit*/ ){
		// Do nothing.
		return;
	}
private:
	bool process(){
		is_eof = code.empty();
		return true;
	}

	string	fname;
	string	code;
	bool	is_eof;
};

void salvia_create_shader( boost::shared_ptr<salviar::shader_code>& scode, std::string const& code, languages lang )
{
	shared_ptr<shader_code_source> code_src( new shader_code_source() );
	if ( !code_src->process_code( code ) ){
		return;
	} 

	shared_ptr<diag_chat> diags = diag_chat::create();

	shared_ptr<program> mroot = sasl::syntax_tree::parse( code_src.get(), code_src, diags.get() );
	if( !mroot ){
		cout << "Syntax error occurs!" << endl;
		return;
	}

	shared_ptr<module_si> msi = analysis_semantic( mroot, diags.get() );
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

	fstream ir_code("for_debug.ll", std::ios_base::out);
	llvmcode->dump( ir_code );
	ir_code.close();

	string errors;

	shared_ptr<shader_code_impl> ret( new shader_code_impl() );
	ret->abii( aa.shared_abii(lang) );
	ret->jit( cgllvm_jit_engine::create( llvmcode, errors ) );
	ret->root( msi->root() );

	scode = ret;
}