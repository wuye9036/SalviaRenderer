#ifndef SASL_SEMANTIC_TYPE_MANAGER_H
#define SASL_SEMANTIC_TYPE_MANAGER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <eflib/include/platform/typedefs.h>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace sasl{
	namespace syntax_tree{
		struct type_specifier;
	}
}

BEGIN_NS_SASL_SEMANTIC();

class symbol;

class type_entry{
public:
	typedef int32_t id_t;
	typedef id_t type_entry::*id_ptr_t;

	type_entry();
	
	::boost::shared_ptr< ::sasl::syntax_tree::type_specifier > stored;
	
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
		::boost::shared_ptr< ::sasl::syntax_tree::type_specifier > node,
		::boost::shared_ptr<symbol> parent
		);
	::boost::shared_ptr< ::sasl::syntax_tree::type_specifier > get( type_entry::id_t id );
private:
	type_entry::id_t allocate_and_assign_id( ::boost::shared_ptr< ::sasl::syntax_tree::type_specifier > node );
	::std::vector< type_entry > entries;
};

END_NS_SASL_SEMANTIC();

#endif