#ifndef SALVIAR_COLOR_CONVERTORS_H
#define SALVIAR_COLOR_CONVERTORS_H
#include <salviar/include/salviar_forward.h>
BEGIN_NS_SALVIAR()


struct color_max{};

template<class T>
struct pixel_type_to_fmt{};

template<int pixel_fmt>
struct pixel_fmt_to_type{};

#define decl_type_fmt_pair(color_type, fmt_code)\
	const pixel_format pixel_format_##color_type = fmt_code;\
	template<>\
	struct pixel_type_to_fmt< color_type >\
	{ static const pixel_format fmt = fmt_code; };\
    \
	template<>\
	struct pixel_fmt_to_type< fmt_code >\
	{ typedef color_type type; };
/*****************************
 *    像素格式的枚举值
 *****************************/
typedef int pixel_format;

/**************************************
 * 编译期枚举值与像素类型的转换
 **************************************/
struct pixel_information
{
	int size;
	char describe[32];
};

#define decl_color_info(pixel_type) \
	{sizeof(pixel_type), #pixel_type}

decl_type_fmt_pair(color_rgba32f, 0);
decl_type_fmt_pair(color_rgb32f, 1);
decl_type_fmt_pair(color_bgra8, 2);
decl_type_fmt_pair(color_rgba8, 3);
decl_type_fmt_pair(color_r32f, 4);
decl_type_fmt_pair(color_rg32f, 5);
decl_type_fmt_pair(color_r32i, 6);
decl_type_fmt_pair(color_max, 7);

const int pixel_format_color_ub = pixel_format_color_max - 1;
/*************************************
 * 颜色信息
 *************************************/
const pixel_information color_infos[pixel_type_to_fmt<color_max>::fmt] = {
	decl_color_info(color_rgba32f),
	decl_color_info(color_rgb32f),
	decl_color_info(color_bgra8),
	decl_color_info(color_rgba8),
	decl_color_info(color_r32f),
	decl_color_info(color_rg32f),
	decl_color_info(color_r32i)
};

inline const pixel_information& get_color_info( pixel_format pf ){
	return color_infos[pf];
}

/**************************************
 * 颜色转换工具
 *************************************/
class pixel_format_convertor
{
	 template <int outColor, int inColor> friend struct color_convertor_initializer;
public:
	static inline void convert(pixel_format outfmt, pixel_format infmt, void* outpixel, const void* inpixel)
	{
		(convertors[outfmt][infmt])(outpixel, inpixel);
	}

	static inline void convert_array(pixel_format outfmt, pixel_format infmt, void* outpixel, const void* inpixel, int count, int outstride = 0, int instride = 0)
	{
		outstride = (outstride == 0) ? color_infos[outfmt].size : outstride;
		instride = (instride == 0) ? color_infos[infmt].size : instride;

		array_convertors[outfmt][infmt](outpixel, inpixel, count, outstride, instride);
	}

	typedef void (*pixel_convertor)(void* outcolor, const void* incolor);
	typedef void (*pixel_array_convertor)(void* outcolor, const void* incolor, int count, int outstride, int instride);
	typedef color_rgba32f (*pixel_lerp_1d)(const void* incolor0, const void* incolor1, float t);
	typedef color_rgba32f (*pixel_lerp_2d)(const void* incolor0, const void* incolor1, const void* incolor2, const void* incolor3, float tx, float ty);

	static pixel_convertor get_convertor_func(pixel_format outfmt, pixel_format infmt)
	{
		return convertors[outfmt][infmt];
	}
	static pixel_array_convertor get_array_convertor_func(pixel_format outfmt, pixel_format infmt)
	{
		return array_convertors[outfmt][infmt];
	}
	static pixel_lerp_1d get_lerp_1d_func(pixel_format infmt)
	{
		return lerpers_1d[infmt];
	}
	static pixel_lerp_2d get_lerp_2d_func(pixel_format infmt)
	{
		return lerpers_2d[infmt];
	}

	pixel_format_convertor();

private:

	//convertors[outfmt][infmt]
	static pixel_convertor convertors[pixel_format_color_max][pixel_format_color_max];
	static pixel_array_convertor array_convertors[pixel_format_color_max][pixel_format_color_max];
	static pixel_lerp_1d lerpers_1d[pixel_format_color_max];
	static pixel_lerp_2d lerpers_2d[pixel_format_color_max];
};

END_NS_SALVIAR()

#endif