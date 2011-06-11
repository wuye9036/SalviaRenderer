#ifndef SOFTART_TYPE_REGISTER_H
#define SOFTART_TYPE_REGISTER_H

#include <eflib/include/math/math.h>
#include <boost/preprocessor/seq/for_each_i.hpp>

	#define BEGIN_REGISTER_TYPE() "type_register.h"

#else
	#if defined(ENUM_TYPE_NAME) || defined(ENUM_ITEM_NAME_GEN)
		#define ENUM_AUTOGEN_ENABLE
	#endif

	#ifdef ENUM_AUTOGEN_ENABLE
		#ifndef ENUM_TYPE_NAME
			#error Need A Definition of ENUM_NAME_FOR_AUTOGEN！
		#endif
		#ifndef ENUM_ITEM_NAME_GEN
			#error Need generator name of enumulation items via ENUM_ITEM_NAME_GEN
		#endif
		#define ENUM_TYPE_NAME ENUM_TYPE_NAME
	#endif

	#ifdef BEGIN_REGISTER_TYPE
		#ifdef END_REGISTER_TYPE
			#error cannot include BEGIN_REGISTER_TYPE 2 times
		#endif
		#define END_REGISTER_TYPE() "type_register.h"
		#undef BEGIN_REGISTER_TYPE
	#else
		//由#include END_REGISTER_TYPE()进来的
		#ifdef END_REGISTER_TYPE
			
			#ifdef BEGIN_REGISTER_TYPE
				#error maybe call the macro END_REGISTER_TYPE twice
			#endif
			
			#ifndef DECL_REGISTERED_TYPES
				#error you must define types for registering by DECL_REGISTERED_TYPES macro!
			#endif

			#define BEGIN_REGISTER_TYPE() "type_register.h"
			
			struct empty{};
			typedef int type_id;
			template<class T> struct type_encode{
				#ifdef ENUM_AUTOGEN_ENABLE
					static const ENUM_TYPE_NAME tag = ENUM_TYPE_NAME(0);
				#endif 
				static const int id = 0; 
			};
			template<int id> struct type_decode{typedef empty type;};

			#ifdef ENUM_AUTOGEN_ENABLE
				#define REGISTER_TYPE(r, dummy, i, type_elem) \
					template<> struct type_encode< type_elem >{static const int id = i; static const ENUM_TYPE_NAME tag = ENUM_TYPE_NAME ( i );};\
					template<> struct type_decode< i >{typedef type_elem type;};
			#else
				#define REGISTER_TYPE(r, dummy, i, type_elem) \
					template<> struct type_encode< type_elem >{static const int id = i; };\
					template<> struct type_decode< i >{typedef type_elem type;};
			#endif

			BOOST_PP_SEQ_FOR_EACH_I(REGISTER_TYPE, _, DECL_REGISTERED_TYPES)
			#undef REGISTER_TYPE

			#ifdef ENUM_AUTOGEN_ENABLE
				#define REGISTER_ENUM_ITEM(r, data, i, type_elem) ENUM_ITEM_NAME_GEN(type_elem) = i,
				enum ENUM_NAME {
					BOOST_PP_SEQ_FOR_EACH_I(REGISTER_ENUM_ITEM, _, DECL_REGISTERED_TYPES)
				};
				#undef REGISTER_ENUM_ITEM
			#endif
			
			const int _registered_types_count = BOOST_PP_SEQ_SIZE(DECL_REGISTERED_TYPES);
			#ifdef ENUM_AUTOGEN_ENABLE
				#undef ENUM_AUTOGEN_ENABLE
				#undef ENUM_TYPE_NAME
				#undef ENUM_ITEM_NAME_GEN
			#endif
			#undef DECL_REGISTERED_TYPES

		#else
			#error check macro!
		#endif
	#endif
#endif