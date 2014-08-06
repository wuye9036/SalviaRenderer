#pragma once

#include <eflib/include/platform/typedefs.h>

#include <utility>
#include <type_traits>

template <typename T>
struct is_enum_class
	: public std::integral_constant<bool, 
		std::is_enum<T>::value && !(std::is_convertible<T, int64_t>::value || std::is_convertible<T, uint64_t>::value)
	>
{
};

template <typename T>
typename std::enable_if< is_enum_class<T>::value, typename std::underlying_type<T>::type >::type
	to_underlying(T v)
{
	typedef typename std::underlying_type<T>::type underlying;
	return static_cast<underlying>(v);
}

template <typename T>
typename std::enable_if<is_enum_class<T>::value, T>::type 
	operator | (T lhs, T rhs)
{
	auto lhs_v = to_underlying(lhs);
	auto rhs_v = to_underlying(rhs);
	return static_cast<T>(lhs_v | rhs_v);
}

template <typename T>
typename std::enable_if<is_enum_class<T>::value, T>::type 
	operator & (T lhs, T rhs)
{
	auto lhs_v = to_underlying(lhs);
	auto rhs_v = to_underlying(rhs);
	return static_cast<T>(lhs_v & rhs_v);
}

template <typename T>
typename std::enable_if<is_enum_class<T>::value, T>::type 
	operator ^ (T lhs, T rhs)
{
	auto lhs_v = to_underlying(lhs);
	auto rhs_v = to_underlying(rhs);
	return static_cast<T>(lhs_v ^ rhs_v);
}

template <typename T>
typename std::enable_if<is_enum_class<T>::value, T>::type 
	operator ~ (T lhs)
{
	return static_cast<T>( ~to_underlying(lhs) );
}

template <typename T, typename U>
typename std::enable_if<std::is_integral<T>::value && is_enum_class<U>::value, T>::type
	operator << (T lhs, U rhs)
{
	return lhs << to_underlying(rhs);
}

template <typename T, typename U>
typename std::enable_if<is_enum_class<T>::value && std::is_integral<U>::value, T>::type
	operator << (T lhs, U rhs)
{
	return static_cast<T>( to_underlying(lhs) << rhs );
}

template <typename T, typename U>
typename std::enable_if<std::is_integral<T>::value && is_enum_class<U>::value, T>::type
	operator >> (T lhs, U rhs)
{
	return lhs >> to_underlying(rhs);
}

template <typename T, typename U>
typename std::enable_if<is_enum_class<T>::value && std::is_integral<U>::value, T>::type
	operator >> (T lhs, U rhs)
{
	return static_cast<T>( to_underlying(lhs) >> rhs );
}
