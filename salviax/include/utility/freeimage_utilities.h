#ifndef SALVIAX_FREEIMAGE_UTILITIES_H
#define SALVIAX_FREEIMAGE_UTILITIES_H

#include <salviax/include/utility/user_config.h>

#ifdef SALVIAX_FREEIMAGE_ENABLED

#include "utility_forward.h"
#include "salviar/include/colors.h"
#include <eflib/include/string/string.h>
#include <eflib/include/math/collision_detection.h>
#include <FreeImage.h>
#include <boost/static_assert.hpp>
#include <algorithm>
#include <tchar.h>

BEGIN_NS_SALVIAX_UTILITY()

FIBITMAP* load_image(const std::_tstring& fname, int flag FI_DEFAULT(0));

// Return true if image type is supported for loading.
bool check_image_type_support(FIBITMAP* image);

// Stretch region of bitmap to another region with specified size.
FIBITMAP* make_bitmap_copy( eflib::rect<size_t>& out_region,
						   size_t dest_width, size_t dest_height,
						   FIBITMAP* image, const eflib::rect<size_t>& src_region );

// Make all colors with different byte orders to the same interface
// to access color components.
template <class ColorType>
struct FREE_IMAGE_UNIFORM_COLOR
{
	STATIC_ASSERT_INFO(FIUC_RECIEVED_A_ILLEGAL_TYPE);
	BOOST_STATIC_ASSERT(FIUC_RECIEVED_A_ILLEGAL_TYPE);

	typedef int CompT;
	const CompT& r;
	const CompT& g;
	const CompT& b;
	const CompT& a;
	FREE_IMAGE_UNIFORM_COLOR(const ColorType& color);
private:
	FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<ColorType>&);
	FREE_IMAGE_UNIFORM_COLOR& operator = (const FREE_IMAGE_UNIFORM_COLOR<ColorType>&);
};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<RGBQUAD>
{
	typedef byte CompT;
	const CompT& r;
	const CompT& g;
	const CompT& b;
	const CompT& a;
	FREE_IMAGE_UNIFORM_COLOR(const CompT* c, CompT /*alpha*/):r(c[FI_RGBA_RED]), g(c[FI_RGBA_GREEN]), b(c[FI_RGBA_BLUE]), a(c[FI_RGBA_ALPHA]){}
private:
	FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<RGBQUAD>&);
	FREE_IMAGE_UNIFORM_COLOR& operator = (const FREE_IMAGE_UNIFORM_COLOR<RGBQUAD>&);
};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<RGBTRIPLE>
{
	typedef byte CompT;
	const CompT& r;
	const CompT& g;
	const CompT& b;
	const CompT& a;
	FREE_IMAGE_UNIFORM_COLOR(const CompT* c, CompT alpha):r(c[FI_RGBA_RED]), g(c[FI_RGBA_GREEN]), b(c[FI_RGBA_BLUE]), a(alpha){}

private:
	FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<RGBTRIPLE>&);
	FREE_IMAGE_UNIFORM_COLOR& operator = (const FREE_IMAGE_UNIFORM_COLOR<RGBTRIPLE>&);
};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<FIRGBF>
{
	typedef float CompT;
	const CompT& r;
	const CompT& g;
	const CompT& b;
	const CompT& a;
	FREE_IMAGE_UNIFORM_COLOR(const CompT* c, CompT alpha):r(c[0]), g(c[1]), b(c[2]), a(alpha){}
private:
	FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<FIRGBF>&);
	FREE_IMAGE_UNIFORM_COLOR& operator = (const FREE_IMAGE_UNIFORM_COLOR<FIRGBF>&);
};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<FIRGBAF>
{
	typedef float CompT;
	const CompT& r;
	const CompT& g;
	const CompT& b;
	const CompT& a;
	FREE_IMAGE_UNIFORM_COLOR(const CompT* c, CompT /*alpha*/):r(c[0]), g(c[1]), b(c[2]), a(c[3]){}
private:
	FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<FIRGBAF>&);
	FREE_IMAGE_UNIFORM_COLOR& operator = (const FREE_IMAGE_UNIFORM_COLOR<FIRGBAF>&);
};

#define FIUC FREE_IMAGE_UNIFORM_COLOR 

// Get the salvia supported color type
// which is compatible with internal color format in FreeImage
// but components order of it is RGBA.
template<typename FIColorT>
struct softart_rgba_color_type{
	typedef salviar::color_max type;
	static const salviar::pixel_format fmt = salviar::pixel_type_to_fmt<typename type>::fmt;
};
template<>
struct softart_rgba_color_type<RGBQUAD>{
	typedef salviar::color_rgba8 type;
	static const salviar::pixel_format fmt = salviar::pixel_type_to_fmt<typename type>::fmt;
};

template<>
struct softart_rgba_color_type<RGBTRIPLE>{
	typedef salviar::color_rgba8 type;
	static const salviar::pixel_format fmt = salviar::pixel_type_to_fmt<typename type>::fmt;
};

template<>
struct softart_rgba_color_type<FIRGBF>{
	typedef salviar::color_rgba32f type;
	static const salviar::pixel_format fmt = salviar::pixel_type_to_fmt<typename type>::fmt;
};

template<>
struct softart_rgba_color_type<FIRGBAF>{
	typedef salviar::color_rgba32f type;
	static const salviar::pixel_format fmt = salviar::pixel_type_to_fmt<typename type>::fmt;
};
END_NS_SALVIAX_UTILITY()

#endif // SOFTART_FREEIMAGE_ENABLED

#endif