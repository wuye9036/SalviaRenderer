#ifndef SASL_TEST_CASES_ATREPO_H
#define SASL_TEST_CASES_ATREPO_H

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

namespace sasl{
	namespace semantic{
		class symbol;
	}
}

// Auto test repository
class at_repo{
	void load( std::string const& path );

	boost::shared_ptr<sasl::semantic::symbol> root();

	void* get_function();
	void* get_entry_function();
private:

};

#endif