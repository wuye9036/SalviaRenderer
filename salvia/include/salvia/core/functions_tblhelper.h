#ifndef FUNCTIONS_TBLHELPER_H
#define FUNCTIONS_TBLHELPER_H

#define FUNC_TABLE(func_impl_name) func_impl_name##_table
#define FUNC_TYPE(func_impl_name) func_impl_name##_type
#define FUNC_INITIALIZER(func_impl_name) BOOST_PP_CAT(FUNC_TABLE(func_impl_name), _initializer)

#define FUNCTIONS_INITIALIZE(rettype, params, func_impl_name, count) \
	typedef rettype (* func_impl_name##_type) params;\
	static FUNC_TYPE(func_impl_name) FUNC_TABLE(func_impl_name)[count+1]; \
	template<int id> \
	struct FUNC_INITIALIZER(func_impl_name) \
	{ \
		FUNC_INITIALIZER(func_impl_name) <id - 1> subinit_; \
		FUNC_INITIALIZER(func_impl_name) () \
		{ \
			FUNC_TABLE(func_impl_name) [id] = & func_impl_name <typename type_decode<id>::type>; \
		} \
	};\
	template<> \
	struct FUNC_INITIALIZER(func_impl_name) <-1> \
	{ \
		FUNC_INITIALIZER(func_impl_name) (){} \
	}; \
	static FUNC_INITIALIZER(func_impl_name)<count> BOOST_PP_CAT(FUNC_INITIALIZER(func_impl_name), _autorunner);

#endif
