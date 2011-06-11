#ifndef SOFTART_SHADER_UTILITY_H
#define SOFTART_SHADER_UTILITY_H

#include <eflib/include/platform/typedefs.h>

#include "type_register.h"
#include "functions_tblhelper.h"
#include <salviar/include/salviar_forward.h>
BEGIN_NS_SALVIAR()


namespace shader_constant
{
	//reg types
	/////////////////////////////////////////////
	#include BEGIN_REGISTER_TYPE()

	#define DECL_REGISTERED_TYPES \
				(empty)\
				(bool)\
				(int8_t)(uint8_t)(int16_t)(uint16_t)(int32_t)(uint32_t)(int64_t)(uint64_t)\
				(std::string)(std::wstring)\
				(eflib::vec2)(eflib::vec3)(eflib::vec4)(eflib::mat44)\
				/*END REGISTER*/

	#include END_REGISTER_TYPE()

	#include "voidptr.h"

	//reg functions
	////////////////////////////////////////////
	template<class T>
	inline bool assign_impl(voidptr lhs, const_voidptr rhs){
		*(T*)lhs = *(const T*)rhs;
		return true;
	}

	template<>
	inline bool assign_impl<empty>(voidptr /*lhs*/, const_voidptr /*rhs*/){
		return false;
	}

	FUNCTIONS_INITIALIZE(bool, (voidptr lhs, const_voidptr rhs), assign_impl, _registered_types_count);

	inline bool assign(voidptr lhs, const_voidptr rhs){
		if(lhs.get_id() == rhs.get_id()){
			return FUNC_TABLE(assign_impl) [lhs.get_id()](lhs, rhs);
		}
		return false;
	}
}

namespace detail
{
	class container
	{
	public:
		virtual void get(shader_constant::voidptr pval, size_t pos) = 0;
		virtual void set(shader_constant::const_voidptr val, size_t pos) = 0;
	};

	template<class ContainerImpl, class ElemType>
	class container_impl : public container
	{
		typedef ContainerImpl container_type;
		typedef ElemType element_type;

		container_type* pcont;

	public:
		container_impl(container_type& cont) : pcont(&cont){
		}

		virtual void get(shader_constant::voidptr pval, size_t pos)
		{
			element_type* pelem = shader_constant::voidptr_cast<element_type>(pval);
			if(pelem){
				*pelem = (*pcont)[pos];
			}
		}

		virtual void set(shader_constant::const_voidptr pval, size_t pos)
		{
			const element_type* pelem = shader_constant::voidptr_cast<element_type>(pval);
			if(pelem){
				(*pcont)[pos] = *pelem;
			}
		}
	};
}
END_NS_SALVIAR()

#endif