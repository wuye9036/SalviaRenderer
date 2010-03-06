#ifndef SASL_DECLARE_HANDLE_H
#define SASL_DECLARE_HANDLE_H

#include <boost/smart_ptr.hpp>
#include <boost/preprocessor/cat.hpp>

#define HANDLE_OF(class_name)\
	BOOST_PP_CAT(h_, class_name)

#define WEAK_HANDLE_OF( class_name ) \
	BOOST_PP_CAT(class_name, *)

template<class T>
struct handle_type_of{
	typedef typename boost::shared_ptr<T> result;
};

#define DECL_HANDLE(class_name) \
	class class_name;\
	typedef handle_type_of< class_name >::result HANDLE_OF(class_name);

#define DECL_STRUCT_HANDLE(struct_name) \
	struct struct_name;\
	typedef handle_type_of< struct_name >::result HANDLE_OF(struct_name);

#endif