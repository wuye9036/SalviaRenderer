#ifndef SALVIAR_HANDLES_H
#define SALVIAR_HANDLES_H

#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
#include <boost/smart_ptr.hpp>
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif


template <class T>
struct handle_maker
{
	typedef boost::shared_ptr<T> handle;
	typedef T* weak_handle;
};

template <class T>
struct maker_of_handle{
};

#define DECL_HANDLE(T, H) \
	typedef ::handle_maker<T>::handle H;

#define DECL_WEAK_HANDLE(T, WH) \
	typedef ::handle_maker<T>::weak_handle WH;

//H is a handle type
template<class H>
typename H::element_type* get_weak_handle(H handle)
{
	return handle.get();
}


#endif