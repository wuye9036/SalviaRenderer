#ifndef SASL_COMMON_COMPILER_INFOS_H
#define SASL_COMMON_COMPILER_INFOS_H

#include <sasl/include/common/common_fwd.h>
#include <sasl/enums/compiler_informations.h>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

BEGIN_NS_SASL_COMMON();

class compiler_information;

class compiler_info_manager{
public:
	static boost::shared_ptr<compiler_info_manager> create();
	typedef std::vector< boost::shared_ptr<compiler_information> > compiler_info_list;

	void add_info( boost::shared_ptr<compiler_information> info );
	void clear();
	compiler_info_list all_condition_infos( compiler_informations filter );
	compiler_info_list one_condition_infos( compiler_informations filter );
private:
	compiler_info_manager();
	compiler_info_manager( const compiler_info_manager& );
	compiler_info_manager& operator = ( const compiler_info_manager& );
	compiler_info_list cinfos;
};

END_NS_SASL_COMMON();

#endif