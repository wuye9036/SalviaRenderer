#pragma once

#include <sasl/enums/builtin_types.h>

namespace sasl{
	namespace utility{
		inline
		builtin_types to_builtin_types( salvia::shader::language_value_types v )
		{
			return static_cast<builtin_types>(v);;
		}
		
		inline
		salvia::shader::language_value_types to_lvt(builtin_types v)
		{
			return static_cast<salvia::shader::language_value_types>(v);
		}
		
		namespace ops{
			inline
			bool operator == ( builtin_types lhs, salvia::shader::language_value_types rhs ){
				return to_lvt(lhs) == rhs;
			}
			inline
			bool operator == ( salvia::shader::language_value_types lhs, builtin_types rhs ){
				return rhs == lhs;
			}
			inline
			bool operator != ( salvia::shader::language_value_types lhs, builtin_types rhs ){
				return !(rhs == lhs);
			}
			inline
			bool operator != ( builtin_types lhs, salvia::shader::language_value_types rhs ){
				return !(lhs == rhs);
			}
		}
	}
}