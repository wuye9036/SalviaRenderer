#include <sasl/test/abi_test/abi_test.h>

#include <sasl/include/drivers/drivers_api.h>
#include <sasl/include/codegen/cg_api.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/common/diag_formatter.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <fstream>

void (*compiler_loader::create_compiler)( shared_ptr<compiler>& );

BOOST_GLOBAL_FIXTURE(compiler_loader);

BOOST_AUTO_TEST_SUITE(death);

BOOST_FIXTURE_TEST_CASE(smoke, abi_test_fixture)
{
	init_vs("repo/semantic_fn.svs");
}

BOOST_AUTO_TEST_SUITE_END();
