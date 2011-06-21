#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sasl/include/common/lex_context.h>
#include <sasl/include/parser/parse_api.h>
#include <sasl/include/syntax_tree/declaration.h>
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
	shared_ptr<sasl::parser::attribute> pt_prog;
	shared_ptr<lex_context> lexctxt( new lex_context_test_impl() );
	std::string code(
		"int a;"
		);

	shared_ptr< ::sasl::syntax_tree::program > prog = ::sasl::syntax_tree::parse( code, lexctxt );
	BOOST_REQUIRE( prog );
	BOOST_REQUIRE( prog->decls.size() == 1 );
	BOOST_REQUIRE( prog->decls[0] );

	using ::sasl::syntax_tree::variable_declaration;
	shared_ptr<variable_declaration > vdecl = prog->decls[0]->typed_handle<variable_declaration>();
	BOOST_REQUIRE( vdecl );
	BOOST_REQUIRE( vdecl->type_info );
	BOOST_CHECK( vdecl->type_info->value_typecode == builtin_types::_sint32 );
	BOOST_REQUIRE( vdecl->declarators.size() == 1 );
	BOOST_CHECK( vdecl->declarators[0]->name->str == "a" );
	BOOST_CHECK( !vdecl->declarators[0]->init );
}

BOOST_AUTO_TEST_SUITE_END();