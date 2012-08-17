#ifndef SALVIAX_RESOURCE_FONT_TEXTURE_H
#define SALVIAX_RESOURCE_FONT_TEXTURE_H

#include <salviax/include/resource/resource_forward.h>

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

namespace eflib
{
	template <typename T> struct rect;
}

namespace salviar
{
	EFLIB_DECLARE_CLASS_SHARED_PTR(surface);
	struct color_rgba32f;
}

BEGIN_NS_SALVIAX_RESOURCE();

EFLIB_DECLARE_CLASS_SHARED_PTR(font);

class font
{
public:
	struct metrics
	{
		int32_t acsent;
		int32_t descent;
		int32_t line_spacing;
	};
	
	struct metrics_in_pixels
	{
		float acsent;
		float descent;
		float line_spacing;
	};
	
	enum styles
	{
		regular 	= 0,
		bold		= 1U << 0,
		italic		= 1U << 1,
		underline	= 1U << 2,
		strokeline	= 1U << 3
	};
	
	enum units
	{
		pixels	= 0,
		points	= 1,
	};
	
	enum alignments
	{
		align_to_near,
		align_to_center,
		aling_to_far
	};
	
	enum render_hints
	{
		bit_pixel,
		antialias,
		cleartype
	};
	
	static font_ptr create(
		std::string const& font_file_path, size_t face_index,
		size_t size, font::units uint );

	static font_ptr create_in_system_path(
		std::string const& font_file_name, size_t face_index,
		size_t size, font::units unit );

	virtual size_t size() const = 0;
	virtual units unit() const = 0;
	virtual void size_and_unit(size_t sz, units un) = 0;
	
	//virtual styles style() const = 0;
	//virtual void style(styles s) = 0;
	//
	//virtual size_t kerning() const = 0;
	//virtual void kerning( size_t sz ) = 0;
	
	virtual void draw(std::string const& text,
		salviar::surface* target, eflib::rect<int32_t> const& rc,
		salviar::color_rgba32f const& foreground_color, salviar::color_rgba32f const& background_color,
		font::render_hints hint) = 0;
};

END_NS_SALVIAX_RESOURCE();

#endif