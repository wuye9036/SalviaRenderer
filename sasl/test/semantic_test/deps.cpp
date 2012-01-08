#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sasl/include/common/lex_context.h>
#include <sasl/include/syntax_tree/parse_api.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/semantic_api.h>

#include <salviar/include/shader_abi.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>
#include <fstream>

using sasl::common::code_source;
using sasl::common::lex_context;
using sasl::syntax_tree::program;
using sasl::semantic::abi_analyser;
using sasl::semantic::module_si;
using sasl::semantic::analysis_semantic;
using boost::shared_ptr;
using boost::make_shared;
using std::fstream;
using std::string;
using std::istream_iterator;
using std::cout;
using std::endl;
using std::ios_base;

class deps_test_code_source: public lex_context, public code_source{
public:
	deps_test_code_source(): eof(true), fname("in_memory"){
	}

	bool process_code( std::string const& code ){
		this->code = code;
		return process();
	}

	// code source
	virtual bool is_eof(){ return eof; }
	virtual std::string next_token(){
		eof = true;
		return code;
	}

	// lex_context
	virtual const std::string& file_name() const{ return fname; }
	virtual size_t column() const{ return 0; }
	virtual size_t line() const{ return 0; }

	virtual void next( const std::string& /*lit*/ ){ return; }
	virtual string error_token(){ return string(""); }
private:
	bool process(){
		eof = code.empty();
		return true;
	}

	std::string fname;
	std::string code;
	bool eof;
};

class deps_fixture
{
public:
	deps_fixture() {}

	void init_g( string const& file_name ){
		init( file_name, salviar::lang_general );
	}

	void init_vs( string const& file_name ){
		init( file_name, salviar::lang_vertex_shader );
	}

	void init_ps( string const& file_name ){
		init( file_name, salviar::lang_pixel_shader );
	}

	void init( string const& file_name, salviar::languages lang ){
		string code;

		fstream code_file;
		code_file.open( file_name, ios_base::in );
		if( !code_file ) cout << "Fatal error: Could not open input file: " << file_name << endl;
		code_file.unsetf(std::ios::skipws);
		std::copy( istream_iterator<char>(code_file), istream_iterator<char>(), std::back_inserter(code) );
		code_file.close();

		shared_ptr<deps_test_code_source> cs = make_shared<deps_test_code_source>();
		cs->process_code( code );

		mroot = sasl::syntax_tree::parse( cs.get(), cs );
		if( !mroot ){
			cout << "Syntax error occurs!" << endl;
			return;
		}

		msi = analysis_semantic( mroot );
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

		BOOST_REQUIRE( mroot );
		BOOST_REQUIRE( msi );
	}

	shared_ptr<program>		mroot;
	shared_ptr<module_si>	msi;
};

BOOST_FIXTURE_TEST_CASE( deps, deps_fixture )
{
	init_ps( "./repo/question/v1a1/intrinsics.sps" );
}