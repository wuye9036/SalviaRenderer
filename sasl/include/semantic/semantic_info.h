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

class semantic_info{
public:
	const std::string& class_name() const{
		return clsname;
	}

	virtual ~semantic_info(){
	}
protected:
	semantic_info( const std::string& cls ):
		 clsname(cls)
	{}
	std::string clsname;
};

class semantic_info_collection{
public:
	template<typename SemanticInfoT> boost::shared_ptr<SemanticInfoT> semantic_info(){
		BOOST_STATIC_ASSERT( (std::tr1::is_base_of<class semantic_info, SemanticInfoT>::value) );

		static ::std::string sem_name = SemanticInfoT().class_name();
		if( seminfos.count( sem_name ) == 0 ){
			return boost::shared_ptr<SemanticInfoT>();
		} else {
			return boost::shared_polymorphic_cast<SemanticInfoT>(seminfos[sem_name]);
		}
	}

	template<typename SemanticInfoT> boost::shared_ptr<SemanticInfoT> get_or_create_semantic_info(){
		BOOST_STATIC_ASSERT( (std::tr1::is_base_of<class semantic_info, SemanticInfoT>::value) );

		boost::shared_ptr<SemanticInfoT> ret( semantic_info<SemanticInfoT>() );
		if (!ret){
			ret.reset( new SemanticInfoT() );
			semantic_info( ret );
		}
		return ret;
	}

	void semantic_info( boost::shared_ptr<class semantic_info> seminfo ){
		if( seminfos.count( seminfo->class_name() ) == 0 ){
			seminfos[seminfo->class_name()] = seminfo;
		}  /* else {
			// do nothing;
		} */
	}
private:
	typedef ::boost::unordered_map<::std::string, boost::shared_ptr<class semantic_info> > semantic_infos_t;
	semantic_infos_t seminfos;
};

template <typename SemanticInfoT, typename NodeU> boost::shared_ptr<SemanticInfoT> extract_semantic_info( boost::shared_ptr<NodeU> pnode ){
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<semantic_info, SemanticInfoT>::value) );
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<node, NodeU>::value) );
	return pnode->semantic_infos()->semantic_info<SemanticInfoT>();
}

template <typename SemanticInfoT, typename NodeU> boost::shared_ptr<SemanticInfoT> extract_semantic_info( NodeU& nd ){
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<semantic_info, SemanticInfoT>::value) );
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<node, NodeU>::value) );
	return nd.semantic_infos()->semantic_info<SemanticInfoT>();
}

template <typename SemanticInfoT, typename NodeU> boost::shared_ptr<SemanticInfoT> get_or_create_semantic_info( boost::shared_ptr<NodeU> pnode ){
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<semantic_info, SemanticInfoT>::value) );
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<node, NodeU>::value) );
	return pnode->semantic_infos()->get_or_create_semantic_info<SemanticInfoT>();
}

template <typename SemanticInfoT, typename NodeU> boost::shared_ptr<SemanticInfoT> get_or_create_semantic_info( NodeU& nd ){
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<semantic_info, SemanticInfoT>::value) );
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<node, NodeU>::value) );
	return nd.semantic_infos()->get_or_create_semantic_info<SemanticInfoT>();
}

END_NS_SASL_SEMANTIC();

#endif