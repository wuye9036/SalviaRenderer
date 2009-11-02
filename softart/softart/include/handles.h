#ifndef SOFTART_HANDLES_H
#define SOFTART_HANDLES_H

#include <boost/smart_ptr.hpp>

template <class T>
struct handle_maker
{
	typedef boost::shared_ptr<T> handle;
	typedef T* weak_handle;

	typedef T data_type;

	static weak_handle get_weak_handle(handle h){
		return h.get();
	}
};

template <class T>
struct maker_of_handle{
};

#define DECL_HANDLE(T, H) \
	typedef handle_maker<T>::handle H;\
	template<> struct maker_of_handle<H>{typedef handle_maker<T> maker_type;}; \

#define DECL_WEAK_HANDLE(T, WH) \
	typedef handle_maker<T>::weak_handle WH;\
	template<> struct maker_of_handle<WH>{typedef handle_maker<T> maker_type;}; \

//H is a handle type
template<class H>
typename maker_of_handle<H>::maker_type::weak_handle get_weak_handle(H handle)
{
	return maker_of_handle<H>::maker_type::get_weak_handle(handle);
}

#endif