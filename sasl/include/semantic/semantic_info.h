#ifndef SASL_SEMANTIC_SEMANTIC_INFO_H
#define SASL_SEMANTIC_SEMANTIC_INFO_H

#include <sasl/include/semantic/semantic_forward.h>
#include <boost/tr1/type_traits.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/unordered_map.hpp>
#include <string>

namespace sasl {
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::node;

/////////////////////////////////////////////////
// storage the semantic of node.
// all of semantic infos are inherited from it.
////////////////////////////////////////////////
class semantic_info{
public:
	virtual ~semantic_info(){}
};

//////////////////////////////////////////////////////
// some utility functions
template <typename SemanticInfoT, typename NodeU>
boost::shared_ptr<SemanticInfoT> extract_semantic_info( boost::shared_ptr<NodeU> pnode ){
	return boost::shared_polymorphic_cast<SemanticInfoT>( pnode->semantic_info() );
}

template <typename SemanticInfoT, typename NodeU> boost::shared_ptr<SemanticInfoT> extract_semantic_info( NodeU& nd ){
	return boost::shared_polymorphic_cast<SemanticInfoT>( nd.semantic_info() );
}

template <typename SemanticInfoT, typename NodeU>
boost::shared_ptr<SemanticInfoT> get_or_create_semantic_info( boost::shared_ptr<NodeU> pnode ){
	assert( pnode );
	if ( !pnode->semantic_info() ){
		pnode->semantic_info( boost::make_shared<SemanticInfoT>() );	
	}
	return extract_semantic_info<SemanticInfoT>(pnode);
}

template <typename SemanticInfoT, typename NodeU> boost::shared_ptr<SemanticInfoT> get_or_create_semantic_info( NodeU& nd ){
	if ( !nd.semantic_info() ){
		nd.semantic_info( boost::make_shared<SemanticInfoT>() );	
	}
	return extract_semantic_info<SemanticInfoT>(nd);
}

END_NS_SASL_SEMANTIC();

#endif