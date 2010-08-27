#ifndef EFLIB_BOOSTEXT_H
#define EFLIB_BOOSTEXT_H

#include <boost/utility.hpp>
#include <boost/preprocessor/cat.hpp>

#define EFLIB_ENABLE_IF_COND( cond, ID )\
	typename ::boost::enable_if< cond >::type* BOOST_PP_CAT( dummy, ID ) = NULL

#define EFLIB_DISABLE_IF_COND( cond, ID )\
	typename ::boost::disable_if< cond >::type* BOOST_PP_CAT( dummy, ID ) = NULL

#define EFLIB_ENABLE_IF_PRED1( pred, T, ID ) \
	typename ::boost::enable_if< BOOST_PP_CAT(::boost::,pred) <T> >::type* BOOST_PP_CAT(dummy, ID) = NULL

#define EFLIB_DISABLE_IF_PRED1( pred, T, ID ) \
	typename ::boost::disable_if< BOOST_PP_CAT(::boost::,pred) <T> >::type* BOOST_PP_CAT(dummy, ID) = NULL

#define EFLIB_ENABLE_IF_PRED2( pred, U, T, ID ) \
	typename ::boost::enable_if< BOOST_PP_CAT(::boost::,pred) <U, T> >::type* BOOST_PP_CAT(dummy, ID) = NULL

#define EFLIB_DISABLE_IF_PRED2( pred, U, T, ID ) \
	typename ::boost::disable_if< BOOST_PP_CAT(::boost::,pred) <U, T> >::type* BOOST_PP_CAT(dummy, ID) = NULL
	
#endif