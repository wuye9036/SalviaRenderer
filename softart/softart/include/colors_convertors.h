#ifndef SOFTART_COLOR_CONVERTORS_H
#define SOFTART_COLOR_CONVERTORS_H

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
		static pixel_format_convertor instance;
		(convertors[outfmt][infmt])(outpixel, inpixel);
	}

	static inline void convert_array(pixel_format outfmt, pixel_format infmt, void* outpixel, const void* inpixel, int count, int outstride = 0, int instride = 0)
	{
		static pixel_format_convertor instance;

		outstride = (outstride == 0) ? color_infos[outfmt].size : outstride;
		instride = (instride == 0) ? color_infos[infmt].size : instride;

		array_convertors[outfmt][infmt](outpixel, inpixel, count, outstride, instride);
	}

	typedef void (*pixel_convertor)(void* outcolor, const void* incolor);
	typedef void (*pixel_array_convertor)(void* outcolor, const void* incolor, int outstride, int instride, int count);

private:
	pixel_format_convertor();

	//convertors[outfmt][infmt]
	static pixel_convertor convertors[pixel_format_color_max][pixel_format_color_max];
	static pixel_array_convertor array_convertors[pixel_format_color_max][pixel_format_color_max];

	template<class OutColorType, class InColorType>
	static void convert(OutColorType* outpixel, const InColorType* inpixel)
	{
		(*outpixel) = (*inpixel);
	}

	template<class OutColorType, class InColorType>
	static void convert_t(void* outpixel, const void* inpixel)
	{
		(*(OutColorType*)outpixel) = (*(const InColorType*)inpixel);
	}

	template<class OutColorType, class InColorType>
	static void convert_array_t(void* outpixel, const void* inpixel, int count, int outstride, int instride)
	{
		byte* o_pbytes = (byte*)outpixel;
		const byte* i_pbytes = (const byte*)inpixel;
		
		for(int i = 0; i < count; ++i){
			*(OutColorType*)(o_pbytes) = *(const InColorType*)(i_pbytes);
			o_pbytes += outstride;
			i_pbytes += instride;
		}
	}
};

#endif