#pragma once

#include <salviar/include/salviar_forward.h>

#include <eflib/include/memory/vls.h>
#include <eflib/include/math/vector.h>

#include <utility>
#include <type_traits>

BEGIN_NS_SALVIAR();

class register_file
{
public:
	static void vls_construct(register_file* rf, size_t stride, register_file const& val)
	{
		for(size_t i_attr = 0; i_attr < stride / sizeof(vec4); ++i_attr)
		{
			rf->attr(i_attr) = val.attr(i_attr);
		}
	}

	static void vls_destroy(register_file* /*rf*/, size_t /*stride*/)
	{
		// do nothing
	}

	eflib::vec4& attr(size_t index)
	{
		return reinterpret_cast<eflib::vec4*>(this)[index];
	}

	eflib::vec4 const& attr(size_t index) const
	{
		return reinterpret_cast<eflib::vec4 const*>(this)[index];
	}
};

END_NS_SALVIAR();

namespace eflib
{
	template <typename T>
	struct vls_traits<
		T,
		typename std::enabled_if<std::is_base_of<salviar::register_file, T>::value>::type
	>
	{
		static bool const need_construct = false;
		static bool const need_destroy   = false;

		static void construct(
			vls_allocator<salviar::register_file>::pointer p,
			vls_allocator<salviar::register_file>::size_type stride,
			vls_allocator<salviar::register_file>::const_reference val)
		{
			for(size_t i_attr = 0; i_attr < stride / sizeof(vec4); ++i_attr)
			{
				p->attr(i_attr) = val.attr(i_attr);
			}
		};
		static void destroy(
			vls_allocator<salviar::register_file>::pointer p,
			vls_allocator<salviar::register_file>::size_type stride);
		static void copy(
			vls_allocator<salviar::register_file>::reference lhs,
			vls_allocator<salviar::register_file>::size_type stride,
			vls_allocator<salviar::register_file>::const_reference rhs)
		{
			for(size_t i_attr = 0; i_attr < stride / sizeof(vec4); ++i_attr)
			{
				lhs.attr(i_attr) = rhs.attr(i_attr);
			}
		}
	};
}
