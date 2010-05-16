#include "../include/colors.h"
BEGIN_NS_SOFTART()

static pixel_format_convertor pfc_instance;

template <int outColor, int inColor>
struct color_convertor_initializer
{
	color_convertor_initializer<outColor, inColor - 1> cci_;
	color_convertor_initializer(pixel_format_convertor::pixel_convertor* pxcvt_table,
		pixel_format_convertor::pixel_array_convertor* pxacvt_table,
		pixel_format_convertor::pixel_lerp_1d* pxlerp_1d_table,
		pixel_format_convertor::pixel_lerp_2d* pxlerp_2d_table)
		:cci_(pxcvt_table, pxacvt_table, pxlerp_1d_table, pxlerp_2d_table){
			pxcvt_table[inColor + outColor * pixel_type_to_fmt<color_max>::fmt] = &pixel_format_convertor::convert_t<pixel_fmt_to_type<outColor>::type, pixel_fmt_to_type<inColor>::type>;
			pxacvt_table[inColor + outColor * pixel_type_to_fmt<color_max>::fmt] = &pixel_format_convertor::convert_array_t<pixel_fmt_to_type<outColor>::type, pixel_fmt_to_type<inColor>::type>;
			pxlerp_1d_table[inColor] = &pixel_format_convertor::lerp_1d_t<pixel_fmt_to_type<inColor>::type>;
			pxlerp_2d_table[inColor] = &pixel_format_convertor::lerp_2d_t<pixel_fmt_to_type<inColor>::type>;
	}
};

template <>
struct color_convertor_initializer<0, 0>
{
	color_convertor_initializer(pixel_format_convertor::pixel_convertor* pxcvt_table,
		pixel_format_convertor::pixel_array_convertor* pxacvt_table,
		pixel_format_convertor::pixel_lerp_1d* pxlerp_1d_table,
		pixel_format_convertor::pixel_lerp_2d* pxlerp_2d_table){
			pxcvt_table[0] = &pixel_format_convertor::convert_t<pixel_fmt_to_type<0>::type, pixel_fmt_to_type<0>::type>;
			pxacvt_table[0] = &pixel_format_convertor::convert_array_t<pixel_fmt_to_type<0>::type, pixel_fmt_to_type<0>::type>;
			pxlerp_1d_table[0] = &pixel_format_convertor::lerp_1d_t<pixel_fmt_to_type<0>::type>;
			pxlerp_2d_table[0] = &pixel_format_convertor::lerp_2d_t<pixel_fmt_to_type<0>::type>;
	}
};

template <int outColor>
struct color_convertor_initializer<outColor, 0>
{
	color_convertor_initializer<outColor - 1, pixel_format_color_max - 1> cci_;
	color_convertor_initializer(pixel_format_convertor::pixel_convertor* pxcvt_table,
		pixel_format_convertor::pixel_array_convertor* pxacvt_table,
		pixel_format_convertor::pixel_lerp_1d* pxlerp_1d_table,
		pixel_format_convertor::pixel_lerp_2d* pxlerp_2d_table)
		:cci_(pxcvt_table, pxacvt_table, pxlerp_1d_table, pxlerp_2d_table){
			pxcvt_table[outColor * pixel_type_to_fmt<color_max>::fmt] = &pixel_format_convertor::convert_t<pixel_fmt_to_type<outColor>::type, pixel_fmt_to_type<0>::type>;
			pxacvt_table[outColor * pixel_type_to_fmt<color_max>::fmt] = &pixel_format_convertor::convert_array_t<pixel_fmt_to_type<outColor>::type, pixel_fmt_to_type<0>::type>;
			pxlerp_1d_table[0] = &pixel_format_convertor::lerp_1d_t<pixel_fmt_to_type<0>::type>;
			pxlerp_2d_table[0] = &pixel_format_convertor::lerp_2d_t<pixel_fmt_to_type<0>::type>;
	}
};

pixel_format_convertor::pixel_convertor pixel_format_convertor::convertors[pixel_format_color_max][pixel_format_color_max];
pixel_format_convertor::pixel_array_convertor pixel_format_convertor::array_convertors[pixel_format_color_max][pixel_format_color_max];
pixel_format_convertor::pixel_lerp_1d pixel_format_convertor::lerpers_1d[pixel_format_color_max];
pixel_format_convertor::pixel_lerp_2d pixel_format_convertor::lerpers_2d[pixel_format_color_max];

pixel_format_convertor::pixel_format_convertor(){
	color_convertor_initializer<pixel_format_color_max - 1, pixel_format_color_max - 1> init_(&convertors[0][0], &array_convertors[0][0],
		&lerpers_1d[0], &lerpers_2d[0]);
}
END_NS_SOFTART()
