#ifndef EFLIB_METAPROG_UTIL_H
#define EFLIB_METAPROG_UTIL_H

template <typename T>
inline void eflib_undef_param( T const& /*x*/ ){
	;
}

#define EFLIB_UNREF_PARAM(x) eflib_undef_param(x)

#define EFLIB_OPERATOR_BOOL( type_name ) \
	typedef void (type_name::*unspecified_bool_type)() const;	\
    void unspecified_bool_type_stub() const {}					\
	operator unspecified_bool_type() const{						\
		return													\
			( do_unspecified_bool() )							\
			? &type_name::unspecified_bool_type_stub : NULL;	\
	}															\
	bool do_unspecified_bool() const							

#define EFLIB_DECLARE_STRUCT_SHARED_PTR(name) struct name; typedef boost::shared_ptr<name> name##_ptr;
#define EFLIB_DECLARE_CLASS_SHARED_PTR(name) class name;  typedef boost::shared_ptr<name> name##_ptr;
#define EFLIB_USING_SHARED_PTR(ns, name) using ns::name; using ns::name##_ptr;

namespace eflib
{
	template <typename T, typename U>
	T polymorphic_cast(U const* v)
	{
		T ret = NULL;

#if defined(EFLIB_DEBUG)
		ret = dynamic_cast<T>(v);
		if(v) { assert(ret); }
#endif

		ret = static_cast<T>(v);
		return ret;
	}

	template <typename T, typename U>
	T polymorphic_cast(U* v)
	{
		T ret = NULL;

#if defined(EFLIB_DEBUG)
		ret = dynamic_cast<T*>(v);
		if(v) { assert(ret); }
#endif

		ret = static_cast<T>(v);
		return ret;
	}

}

#endif
