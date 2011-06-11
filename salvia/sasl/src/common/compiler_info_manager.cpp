#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/common/compiler_information.h>

BEGIN_NS_SASL_COMMON();

boost::shared_ptr<compiler_info_manager> compiler_info_manager::create(){
	return boost::shared_ptr<compiler_info_manager>( new compiler_info_manager );
}

compiler_info_manager::compiler_info_manager(){
}

void compiler_info_manager::add_info( boost::shared_ptr<compiler_information> info ){
	cinfos.push_back( info );
}

void compiler_info_manager::clear(){
	cinfos.clear();
}

compiler_info_manager::compiler_info_list compiler_info_manager::all_condition_infos( compiler_informations filter ){
	compiler_info_list retlst;
	for( compiler_info_list::iterator it = cinfos.begin(); it != cinfos.end(); ++it ){
		if( (*it)->id(filter) == filter ){
			retlst.push_back( *it );
		}
	}
	return retlst;
}

compiler_info_manager::compiler_info_list compiler_info_manager::one_condition_infos( compiler_informations filter ){
	compiler_info_list retlst;
	for( compiler_info_list::iterator it = cinfos.begin(); it != cinfos.end(); ++it ){
		if( (*it)->id(filter) != compiler_informations::none ){
			retlst.push_back( *it );
		}
	}
	return retlst;
}

END_NS_SASL_COMMON();