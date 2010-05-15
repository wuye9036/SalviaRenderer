#include <boost/test/unit_test.hpp>
#include <sasl/enums/compiler_informations.h>
#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/semantic/semantic_error.h>

BOOST_AUTO_TEST_SUITE( compiler_information_test )

BOOST_AUTO_TEST_CASE( compiler_information ){
	using ::sasl::common::compiler_info_manager;
	using ::sasl::semantic::errors::semantic_error;

	boost::shared_ptr< compiler_info_manager > cim = compiler_info_manager::create();
	cim->add_info( semantic_error::create( compiler_informations::redef_diff_basic_type ) );
	compiler_info_manager::compiler_info_list cil_cw = cim->all_condition_infos( compiler_informations::_compile | compiler_informations::_warning );
	compiler_info_manager::compiler_info_list cil_ce = cim->all_condition_infos( compiler_informations::_compile | compiler_informations::_error );
	compiler_info_manager::compiler_info_list cil_lw = cim->all_condition_infos( compiler_informations::_link | compiler_informations::_warning );
	compiler_info_manager::compiler_info_list cil_c = cim->all_condition_infos( compiler_informations::_compile );
	compiler_info_manager::compiler_info_list cil_l = cim->all_condition_infos( compiler_informations::_link );

	BOOST_CHECK_EQUAL( cil_cw.size(), 0 );
	BOOST_CHECK_EQUAL( cil_ce.size(), 1 );
	BOOST_CHECK_EQUAL( cil_lw.size(), 0 );
	BOOST_CHECK_EQUAL( cil_c.size(), 1 );
	BOOST_CHECK_EQUAL( cil_l.size(), 0 );
}

BOOST_AUTO_TEST_SUITE_END()