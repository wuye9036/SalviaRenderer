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

using namespace salviar;

void (*compiler_loader::create_compiler)( shared_ptr<compiler>& );

BOOST_GLOBAL_FIXTURE(compiler_loader);

BOOST_AUTO_TEST_SUITE(death);

BOOST_FIXTURE_TEST_CASE(semantic_fn_svs, abi_test_fixture)
{
	init_vs("repo/semantic_fn.svs");

	auto reflection2 = drv->get_reflection2();
	BOOST_REQUIRE(reflection2);

	BOOST_CHECK_EQUAL(reflection2->language(), salviar::lang_vertex_shader);
	BOOST_CHECK_EQUAL(reflection2->entry_name(), "Mfn@@");
	BOOST_CHECK_EQUAL(reflection2->available_reg_count(salviar::reg_categories::unknown), 0);
	BOOST_CHECK_EQUAL(reflection2->available_reg_count(salviar::reg_categories::offset), 0);
	BOOST_CHECK_EQUAL(reflection2->available_reg_count(salviar::reg_categories::outputs), 1);
	BOOST_CHECK_EQUAL(reflection2->available_reg_count(salviar::reg_categories::uniforms), 2);
	BOOST_CHECK_EQUAL(reflection2->available_reg_count(salviar::reg_categories::varying), 0);
}

BOOST_AUTO_TEST_SUITE_END();
