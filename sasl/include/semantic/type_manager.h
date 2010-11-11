#ifndef SASL_SEMANTIC_TYPE_MANAGER_H
#define SASL_SEMANTIC_TYPE_MANAGER_H

#include <sasl/include/semantic/forward.h>

#include <boost/shared_ptr.hpp>
#include <vector>

BEGIN_NS_SASL_SEMANTIC();

class type_entry{
public:
	typedef int32_t id_t;
	
	type_entry();
	
	::boost::shared_ptr< type_specifier > stored;
	
	id_t c_qual;
	id_t u_qual;
	/*
	id_t v_qual;
	id_t cv_qual;
	id_t a_qual;
	*/
};

class type_manager{
public:
	type_entry::id_t get(
		::boost::shared_ptr<type_specifier> node,
		::boost::shared_ptr<symbol> parent
		);
	
private:
	::std::vector< type_entry > entries;
};

END_NS_SASL_SEMANTIC();

#endif