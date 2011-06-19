#include <salviar/include/shader_code.h>

#include <sasl/include/common/lex_context.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>
#include <sasl/include/semantic/abi_info.h>
#include <sasl/include/syntax_tree/parse_api.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/semantic/semantic_api.h>
#include <sasl/include/semantic/abi_analyser.h>

#include <iostream>
#include <fstream>

using namespace sasl::semantic;
using boost::shared_ptr;
using std::cout;
using std::endl;
using std::fstream;

BEGIN_NS_SALVIAR();

shader_code::shader_code(): pfn(NULL){
}

void shader_code::abii( shared_ptr<sasl::semantic::abi_info> const& v ){
	shader_abii = v;
}

sasl::semantic::abi_info const* shader_code::abii() const{
	return shader_abii.get();
}

void shader_code::jit( boost::shared_ptr<sasl::code_generator::jit_engine> const& v ){
	je = v;
}

void* shader_code::function_pointer() const{
	return pfn;
}

void shader_code::update(){
	assert( shader_abii && je );
	if( !shader_abii || !je ){
		pfn = NULL;
		return;
	}
	pfn = je->get_function( shader_abii->entry_name() );
}


END_NS_SALVIAR();

using namespace salviar;
using namespace sasl::code_generator;
using namespace sasl::common;
using namespace sasl::syntax_tree;

using std::string;
using boost::shared_static_cast;

class shader_code_source: public lex_context, public code_source{
public:
	shader_code_source(): eof(true), fname("in_memory"){
	}

	bool process_code( std::string const& code ){
		this->code = code;
		return process();
	}

	// code source
	virtual bool is_eof(){
		return eof;
	}

	virtual std::string next_token(){
		eof = true;
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

	virtual void next( const std::string& /*lit*/ ){
		// Do nothing.
		return;
	}

	virtual string error_token(){
		return string("");
	}
private:
	bool process(){
		eof = code.empty();
		return true;
	}

	std::string fname;
	std::string code;
	bool eof;
};

void salvia_create_shader( boost::shared_ptr<salviar::shader_code>& scode, std::string const& code, languages lang )
{
	shared_ptr<shader_code_source> code_src( new shader_code_source() );
	if ( !code_src->process_code( code ) ){
		cout << "Fatal error: Could not process code:  \n\t" << code << endl;
		return;
	} 

	shared_ptr<program> mroot = sasl::syntax_tree::parse( code_src.get(), code_src );
	if( !mroot ){
		cout << "Syntax error occurs!" << endl;
		return;
	}

	shared_ptr<module_si> msi = analysis_semantic( mroot );
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
	dump( llvmcode, ir_code );
	ir_code.close();

	string errors;

	shared_ptr<shader_code> ret( new shader_code() );
	ret->abii( aa.shared_abii(lang) );
	ret->jit( cgllvm_jit_engine::create( llvmcode, errors ) );
	ret->update();

	scode = ret;
}
