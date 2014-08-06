#ifndef SASL_HOST_UTILITY_H
#define SASL_HOST_UTILITY_H

#include <sasl/enums/builtin_types.h>
#include <salviar/include/input_layout.h>

namespace sasl{
	namespace utility{
		inline
		builtin_types to_builtin_types( salviar::language_value_types v )
		{
			return static_cast<builtin_types>(v);;
		}
		
		inline
		salviar::language_value_types to_lvt(builtin_types v)
		{
			return static_cast<salviar::language_value_types>(v);
		}
		
		namespace ops{
			inline
			bool operator == ( builtin_types lhs, salviar::language_value_types rhs ){
				return to_lvt(lhs) == rhs;
			}
			inline
			bool operator == ( salviar::language_value_types lhs, builtin_types rhs ){
				return rhs == lhs;
			}
			inline
			bool operator != ( salviar::language_value_types lhs, builtin_types rhs ){
				return !(rhs == lhs);
			}
			inline
			bool operator != ( builtin_types lhs, salviar::language_value_types rhs ){
				return !(lhs == rhs);
			}
		}
	}
}
#endif