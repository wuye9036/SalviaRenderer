#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>
#include <salviar/include/colors.h>
#include <salviar/include/surface.h>
#include <salviar/include/renderer_capacity.h>

#include <eflib/include/math/math.h>
#include <eflib/include/memory/allocator.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/array.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

struct viewport;

class vs_input
{
public:
	vs_input()
	{}

	eflib::vec4& attribute(size_t index)
	{
		return attributes_[index];
	}

	eflib::vec4 const& attribute(size_t index) const
	{
		return attributes_[index];
	}

private:
	typedef boost::array<
		eflib::vec4, MAX_VS_INPUT_ATTRS > attribute_array;
	attribute_array attributes_;

	vs_input(const vs_input& rhs);
	vs_input& operator=(const vs_input& rhs);
};

#include <eflib/include/platform/disable_warnings.h>
class EFLIB_ALIGN(16) vs_output
{
public:
	/*
	BUG FIX:
		Type A is aligned by A bytes, new A is A bytes aligned too, but address of new A[] is not aligned.
		It is known bug but not fixed yet.
		operator new/delete will ensure the address is aligned.
	*/
	static void* operator new[] (size_t size)
	{
		if(size == 0) { size = 1; }
		return eflib::aligned_malloc(size, 16);
	}

	static void operator delete[](void* p)
	{
		if(p == nullptr) return;
		return eflib::aligned_free(p);
	}

	enum attrib_modifier_type
	{
		am_linear = 1UL << 0,
		am_centroid = 1UL << 1,
		am_nointerpolation = 1UL << 2,
		am_noperspective = 1UL << 3,
		am_sample = 1UL << 4
	};

public:
	eflib::vec4& position()
	{
		return registers_[0];
	}

	eflib::vec4 const& position() const
	{
		return registers_[0];
	}

	eflib::vec4* attribute_data()
	{
		return registers_.data() + 1;
	}

	eflib::vec4 const* attribute_data() const
	{
		return registers_.data() + 1;
	}

	eflib::vec4* raw_data()
	{
		return registers_.data();
	}

	eflib::vec4 const* raw_data() const
	{
		return registers_.data();
	}

	eflib::vec4 const& attribute(size_t index) const
	{
		return attribute_data()[index];
	}

	eflib::vec4& attribute(size_t index)
	{
		return attribute_data()[index];
	}

	vs_output()
	{}

private:
	typedef boost::array<
		eflib::vec4, MAX_VS_OUTPUT_ATTRS+1 > register_array;
	register_array registers_;

	vs_output(const vs_output& rhs);
	vs_output& operator=(const vs_output& rhs);
};
#include <eflib/include/platform/enable_warnings.h>

struct triangle_info
{
	vs_output const*			v0;
	bool						front_face;
	EFLIB_ALIGN(16)	eflib::vec4	bounding_box;
	EFLIB_ALIGN(16)	eflib::vec4	edge_factors[3];
	vs_output					ddx;
	vs_output					ddy;

	triangle_info() {}
	triangle_info(triangle_info const& /*rhs*/)
	{
	}
	triangle_info& operator = (triangle_info const& /*rhs*/)
	{
		return *this;
	}
};

//vs_output compute_derivate
struct ps_output
{
	boost::array<eflib::vec4, MAX_RENDER_TARGETS> color;
};

struct pixel_accessor
{
	pixel_accessor(surface** const& color_buffers, surface* ds_buffer)
    {
        color_buffers_ = color_buffers;
        ds_buffer_ = ds_buffer;
	}

	void set_pos(size_t x, size_t y)
    {
		x_ = x;
		y_ = y;
	}

	color_rgba32f color(size_t target_index, size_t sample_index) const
	{
		if(color_buffers_[target_index] == nullptr)
		{
			return color_rgba32f(0.0f, 0.0f, 0.0f, 0.0f);
		}
		return color_buffers_[target_index]->get_texel(x_, y_, sample_index);
	}

	void color(size_t target_index, size_t sample, const color_rgba32f& clr)
	{
        if(color_buffers_[target_index] != nullptr)
		{
			color_buffers_[target_index]->set_texel(x_, y_, sample, clr);
		}
	}

    void* depth_stencil_address(size_t sample) const
	{
        return ds_buffer_->texel_address(x_, y_, sample);
	}

private:
	pixel_accessor(const pixel_accessor& rhs);
	pixel_accessor& operator = (const pixel_accessor& rhs);

    surface**   color_buffers_;
	surface*    ds_buffer_;
	size_t      x_, y_;
};

END_NS_SALVIAR();
