#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sasl/include/common/lex_context.h>
#include <sasl/include/parser/parse_api.h>
#include <sasl/include/parser_tree/program.h>
#include <sasl/include/syntax_tree/parse_api.h>
#include <sasl/include/syntax_tree/program.h>

using ::sasl::common::lex_context;
using ::boost::shared_ptr;

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

BOOST_AUTO_TEST_SUITE( parser );

BOOST_AUTO_TEST_CASE( program_test ){
	::sasl::parser_tree::program pt_prog;
	shared_ptr<lex_context> lexctxt( new lex_context_test_impl() );
	std::string code(
		"int a;"
		);

	::sasl::parser::parse( pt_prog, code, lexctxt );
	BOOST_REQUIRE( pt_prog.size() == 1 );

	boost::shared_ptr< ::sasl::syntax_tree::program > prog = ::sasl::syntax_tree::parse( code, lexctxt );
	BOOST_REQUIRE( prog );
}

BOOST_AUTO_TEST_SUITE_END();