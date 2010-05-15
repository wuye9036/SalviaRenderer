#ifndef SASL_SEMANTIC_SEMANTIC_ERROR_H
#define SASL_SEMANTIC_SEMANTIC_ERROR_H

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/include/common/compiler_information.h>
#include <boost/shared_ptr.hpp>
#include <vector>

#define BEGIN_NS_SASL_SEMANTIC_ERRORS() namespace sasl{ namespace semantic{ namespace errors{
#define END_NS_SASL_SEMANTIC_ERRORS() } } }

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC_ERRORS();

class semantic_error: public ::sasl::common::compiler_information_impl{
public:
	static boost::shared_ptr<semantic_error> create(
		compiler_informations info,
		boost::shared_ptr<::sasl::syntax_tree::node> info_node = boost::shared_ptr<::sasl::syntax_tree::node>(),
		std::vector< boost::shared_ptr<::sasl::syntax_tree::node> > refnodes = std::vector< boost::shared_ptr<::sasl::syntax_tree::node> >()
		);
	template <typename ListOfT>
	static boost::shared_ptr<semantic_error> create(
		compiler_informations info,
		boost::shared_ptr<::sasl::syntax_tree::node> info_node,
		const ListOfT& list_of_refnode
		){
		boost::shared_ptr<semantic_error> ret = create( info, info_node );
		list_of_refnode.to_container( ret->refnodes );
		return ret;
	}
	static std::vector< boost::shared_ptr<::sasl::syntax_tree::node> > new_ref_node_list();

	virtual std::string desc();

	virtual void add_refnode( boost::shared_ptr<::sasl::syntax_tree::node> refnode );
	virtual std::vector< boost::shared_ptr<::sasl::syntax_tree::node> >& ref_nodes();
protected:
	semantic_error(
		compiler_informations info,
		boost::shared_ptr<::sasl::syntax_tree::node> error_node,
		std::vector< boost::shared_ptr<::sasl::syntax_tree::node> > refnodes
		);
	semantic_error( const semantic_error& );
	semantic_error& operator = ( const semantic_error& );

	std::vector<boost::shared_ptr<::sasl::syntax_tree::node> > refnodes;
	boost::shared_ptr<::sasl::syntax_tree::node> error_node;
};

END_NS_SASL_SEMANTIC_ERRORS();

#endif