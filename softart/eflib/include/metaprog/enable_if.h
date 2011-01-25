#ifndef EFLIB_METAPROG_ENABLE_IF_H
#define EFLIB_METAPROG_ENABLE_IF_H

#include <boost/utility.hpp>
#include <boost/preprocessor/cat.hpp>

#define EFLIB_ENABLE_IF_COND( cond )\
	typename ::boost::enable_if< cond >::type* = NULL

#define EFLIB_DISABLE_IF_COND( cond )\
	typename ::boost::disable_if< cond >::type* = NULL

#define EFLIB_ENABLE_IF_PRED1( pred, T ) \
	typename ::boost::enable_if< BOOST_PP_CAT(::boost::,pred) <T> >::type* = NULL

#define EFLIB_DISABLE_IF_PRED1( pred, T ) \
	typename ::boost::disable_if< BOOST_PP_CAT(::boost::,pred) <T> >::type* = NULL

#define EFLIB_ENABLE_IF_PRED2( pred, U, T ) \
	typename ::boost::enable_if< ::boost:: pred <U, T> >::type* = NULL

#define EFLIB_DISABLE_IF_PRED2( pred, U, T ) \
	typename ::boost::disable_if< BOOST_PP_CAT(::boost::,pred) <U, T> >::type* = NULL
	
#endif