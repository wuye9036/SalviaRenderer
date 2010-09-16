#ifndef SASL_CODE_GENERATOR_CODE_GEN_CONTEXT_H
#define SASL_CODE_GENERATOR_CODE_GEN_CONTEXT_H

#include <sasl/include/code_generator/forward.h>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace sasl{ namespace syntax_tree{ struct node; } }

BEGIN_NS_SASL_CODE_GENERATOR();

///////////////////////////////////////////////////////////
// the base class of all code generation context object
// It could be attached to the syntax node.
class codegen_context{
public:
	boost::shared_ptr<struct ::sasl::syntax_tree::node> node() const{ return owner.lock(); }
	void node( boost::shared_ptr<struct ::sasl::syntax_tree::node> n ){ owner = n; }
	
	virtual ~codegen_context(){}
protected:
	codegen_context(){}
	boost::weak_ptr<struct ::sasl::syntax_tree::node> owner;
};

template <typename T, typename NodeT>
boost::shared_ptr<T> create_codegen_context( boost::shared_ptr<NodeT> nd ){
	boost::shared_ptr<T> ret = boost::make_shared<T>();
	ret->node( nd );
	nd->codegen_ctxt( ret );
	return ret;
}

template <typename CodegenContextT, typename NodeU>
boost::shared_ptr<CodegenContextT> extract_codegen_context( boost::shared_ptr<NodeU> pnode ){
	assert( pnode );
	if ( pnode->codegen_ctxt() ){
		return boost::shared_polymorphic_cast<CodegenContextT>( pnode->codegen_ctxt() );
	}
	return boost::shared_ptr<CodegenContextT>();
}

template <typename CodegenContextT, typename NodeU> boost::shared_ptr<CodegenContextT> extract_codegen_context( NodeU& nd ){
	if ( nd.codegen_ctxt() ){
		return boost::shared_polymorphic_cast<CodegenContextT>( nd.codegen_ctxt() );
	}
	return boost::shared_ptr<CodegenContextT>();
}

template <typename CodegenContextT, typename NodeU>
boost::shared_ptr<CodegenContextT> get_or_create_codegen_context( boost::shared_ptr<NodeU> pnode ){
	assert( pnode );
	if ( !pnode->codegen_ctxt() ){
		create_codegen_context<CodegenContextT>( pnode );
	}
	return extract_codegen_context<CodegenContextT>(pnode);
}

template <typename CodegenContextT, typename NodeU> boost::shared_ptr<CodegenContextT> get_or_create_codegen_context( NodeU& nd ){
	if ( !nd.codegen_ctxt() ){
		create_codegen_context<CodegenContextT>( nd.handle() );
	}
	return extract_codegen_context<CodegenContextT>(nd);
}
END_NS_SASL_CODE_GENERATOR();

#endif // SASL_CODE_GENERATOR_CODEGEN_CONTEXT_H