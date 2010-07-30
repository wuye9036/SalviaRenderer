#include <sasl/include/syntax_tree/make_tree.h>

#include <sasl/enums/buildin_type_code.h>
#include <sasl/include/common/token_attr.h>
#include <sasl/include/syntax_tree/declaration.h>

#include <boost/static_assert.hpp>
//#include <boost/test/unit_test.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_same.hpp>

BEGIN_NS_SASL_SYNTAX_TREE();

using ::sasl::common::token_attr;

boost::shared_ptr<token_attr> null_token(){
	return boost::shared_ptr<token_attr>();
}

boost::shared_ptr<token_attr> make_tree( const ::std::string& lit ){
	return boost::shared_ptr<token_attr>( new token_attr( lit.begin(), lit.end() ) );
}

boost::shared_ptr<buildin_type> make_tree( const buildin_type_code btc ){
	boost::shared_ptr<buildin_type> ret = create_node<buildin_type>( null_token() );
	ret->value_typecode = btc;
	return ret;
}

END_NS_SASL_SYNTAX_TREE();

struct empty_type{
	static boost::shared_ptr<empty_type> null(){
		return boost::shared_ptr<empty_type>();
	}
};

struct no_matched{
};
// do nothing...
no_matched make_tree( ... ){
	return no_matched();
}

template<typename U, typename T>
bool is_same_type( T, EFLIB_ENABLE_IF( is_same, U, T, 0 ) ){
	return true;
}

template<typename U, typename T>
bool is_same_type( T, EFLIB_DISABLE_IF( is_same, U, T, 0 ) ){
	return false;
}

template <typename T>
T& null_instance(){
	return *((T*)NULL);
}



//BOOST_AUTO_TEST_SUITE( tree_maker )
//
//BOOST_AUTO_TEST_CASE( make_tree_test ){
//	::std::string litstr("_test_case_only_");
//	BOOST_CHECK_EQUAL( make_tree( litstr )->lit, litstr );
//
//	buildin_type_code dbltc( buildin_type_code::_double );
//	boost::shared_ptr<sasl::syntax_tree::buildin_type> dbltype = make_tree( dbltc );
//	BOOST_CHECK( dbltype->value_typecode == dbltc );
//	
//	boost::shared_ptr<sasl::syntax_tree::variable_declaration> dblvar = make_tree( dbltype, litstr );
//	BOOST_CHECK( !dblvar->init );
//	BOOST_CHECK( dblvar->name->lit == litstr );
//	BOOST_CHECK( dblvar->type_info->value_typecode == dbltc );	
//
//	BOOST_CHECK( is_same_type<no_matched>( make_tree( empty_type::null(), litstr ) ) );
//}
//
//BOOST_AUTO_TEST_SUITE_END()