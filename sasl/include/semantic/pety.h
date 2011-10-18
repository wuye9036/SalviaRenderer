#ifndef SASL_SEMANTIC_TYPE_MANAGER_H
#define SASL_SEMANTIC_TYPE_MANAGER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <eflib/include/platform/typedefs.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl{
	namespace syntax_tree{
		struct tynode;
	}
}

struct builtin_types;

BEGIN_NS_SASL_SEMANTIC();

typedef int tid_t;

class symbol;

class pety_item_t{
public:
	typedef int pety_item_t::*id_ptr_t;
	pety_item_t();
	
	::boost::shared_ptr< ::sasl::syntax_tree::tynode > stored;
	tid_t u_qual;
};

class pety_t{
public:
	static boost::shared_ptr< pety_t > create();

	void root_symbol( boost::shared_ptr<symbol> const& sym );

	boost::shared_ptr<pety_t> handle() const;
	
	tid_t get(
		::boost::shared_ptr< ::sasl::syntax_tree::tynode > const& node,
		::boost::shared_ptr<symbol> const& parent
		);
	tid_t get( const builtin_types& btc );

	::boost::shared_ptr< ::sasl::syntax_tree::tynode > get( tid_t id );
	
private:
	tid_t allocate_and_assign_id( ::boost::shared_ptr< ::sasl::syntax_tree::tynode > const& node );
	::std::vector< pety_item_t > entries;
	boost::weak_ptr<pety_t> self_handle;
	boost::weak_ptr<symbol> rootsym;
};

END_NS_SASL_SEMANTIC();

#endif