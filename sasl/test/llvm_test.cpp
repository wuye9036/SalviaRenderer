#define BOOST_TEST_MODULE Code Generator Test
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
    int ret = ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
	system("pause");
	return ret;
}
