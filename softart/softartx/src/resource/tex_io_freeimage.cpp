/*
Copyright (C) 2004-2005 Minmin Gong

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "softartx/include/resource/tex_io_freeimage.h"

#include "softart/include/renderer_impl.h"
#include "softart/include/resource_manager.h"

#include "eflib/include/eflib.h"

#include <FreeImage.h>

#include <tchar.h>

#include <boost/static_assert.hpp>
#include <algorithm>

using namespace efl;
using namespace std;

FIBITMAP* load_image(const std::string& filename, int flag FI_DEFAULT(0))
{
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename.c_str(), 0);
	if(fif == FIF_UNKNOWN){
		fif = FreeImage_GetFIFFromFilename(filename.c_str());
	}
	if( fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif) ){
		return FreeImage_Load(fif, filename.c_str(), flag);
	}
	return NULL;
}

bool check_image_type_support(FIBITMAP* image)
{
	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(image);

	switch(image_type){
		case FIT_UNKNOWN:
		case FIT_UINT16:
		case FIT_INT16:
		case FIT_UINT32:
		case FIT_INT32:
		case FIT_FLOAT:
		case FIT_DOUBLE:
		case FIT_COMPLEX:
		case FIT_RGB16:
		case FIT_RGBA16:
		case FIT_RGBF:
			return false;
		case FIT_RGBAF:
			return true;
		case FIT_BITMAP:
			if(FreeImage_GetColorType(image) == FIC_RGB){
				size_t bpp = FreeImage_GetBPP(image);
				if(bpp == 24 || bpp == 32) return true;
				return false;
			}

			if(FreeImage_GetColorType(image) == FIC_RGBALPHA){
				return true;
			}
	}
	return false;
}

//不依赖字节序，统一FreeImage Color的访问方式。
template <class ColorType>
struct FREE_IMAGE_UNIFORM_COLOR
{
	STATIC_ASSERT_INFO(FIUC_RECIEVED_A_ILLEGAL_TYPE);
	BOOST_STATIC_ASSERT(FIUC_RECIEVED_A_ILLEGAL_TYPE);

	typedef int comp_t;
	const comp_t& r;
	const comp_t& g;
	const comp_t& b;
	const comp_t& a;
	FREE_IMAGE_UNIFORM_COLOR(const ColorType& color);
private:
	FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<ColorType>&);
	FREE_IMAGE_UNIFORM_COLOR& operator = (const FREE_IMAGE_UNIFORM_COLOR<ColorType>&);
};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<RGBQUAD>
{
	typedef byte comp_t;
	const comp_t& r;
	const comp_t& g;
	const comp_t& b;
	const comp_t& a;
	FREE_IMAGE_UNIFORM_COLOR(const comp_t* c, comp_t /*alpha*/):r(c[FI_RGBA_RED]), g(c[FI_RGBA_GREEN]), b(c[FI_RGBA_BLUE]), a(c[FI_RGBA_ALPHA]){}
private:
	FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<RGBQUAD>&);
	FREE_IMAGE_UNIFORM_COLOR& operator = (const FREE_IMAGE_UNIFORM_COLOR<RGBQUAD>&);
};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<RGBTRIPLE>
{
	typedef byte comp_t;
	const comp_t& r;
	const comp_t& g;
	const comp_t& b;
	const comp_t& a;
	FREE_IMAGE_UNIFORM_COLOR(const comp_t* c, comp_t alpha):r(c[FI_RGBA_RED]), g(c[FI_RGBA_GREEN]), b(c[FI_RGBA_BLUE]), a(alpha){}

private:
	FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<RGBTRIPLE>&);
	FREE_IMAGE_UNIFORM_COLOR& operator = (const FREE_IMAGE_UNIFORM_COLOR<RGBTRIPLE>&);
};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<FIRGBF>
{
	typedef float comp_t;
	const comp_t& r;
	const comp_t& g;
	const comp_t& b;
	const comp_t& a;
	FREE_IMAGE_UNIFORM_COLOR(const comp_t* c, comp_t alpha):r(c[0]), g(c[1]), b(c[2]), a(alpha){}
private:
	FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<FIRGBF>&);
	FREE_IMAGE_UNIFORM_COLOR& operator = (const FREE_IMAGE_UNIFORM_COLOR<FIRGBF>&);
};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<FIRGBAF>
{
	typedef float comp_t;
	const comp_t& r;
	const comp_t& g;
	const comp_t& b;
	const comp_t& a;
	FREE_IMAGE_UNIFORM_COLOR(const comp_t* c, comp_t /*alpha*/):r(c[0]), g(c[1]), b(c[2]), a(c[3]){}
private:
	FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<FIRGBAF>&);
	FREE_IMAGE_UNIFORM_COLOR& operator = (const FREE_IMAGE_UNIFORM_COLOR<FIRGBAF>&);
};

#define FIUC FREE_IMAGE_UNIFORM_COLOR 

//以rgba的方式copy纹理；
//	mediator_type:	颜色分量类型与源图片像素相同的RGBA顺序的颜色(rgba8, rgba16, rgba32f等)
//	color_t:				FreeImage中颜色的实际存储类型。如RGBQUAD, RGBTRIPLE, FIRGBAF
//	comp_t:				分量类型。例如RGBA8中即为byte

//e.g. :
//copy_image_to_texture_rgba<color_rgba8, RGBTRIPLE, BYTE>(...)
template <class mediator_type, class color_t, class comp_t>
bool copy_image_to_texture_rgba(
								texture* ptex,
								size_t miplevel,
								FIBITMAP* image,
								const rect<size_t>& src_rect,
								comp_t alpha = 0
								)
{
	custom_assert(
		src_rect.w == ptex->get_width(miplevel) &&
		src_rect.h == ptex->get_height(miplevel), 
		""
		);

	byte* pdata = NULL;
	ptex->lock((void**)&pdata, miplevel, rect<size_t>(0, 0, src_rect.w, src_rect.h), lock_write_only);
	if(!pdata) {
		return false;
	}

	BYTE* image_data = FreeImage_GetBits(image);

	size_t pitch = FreeImage_GetPitch(image);
	size_t Bpp = (FreeImage_GetBPP(image) >> 3);

	image_data += (pitch * src_rect.y);
	image_data += Bpp * src_rect.x;

	for(size_t iheight = 0; iheight < src_rect.h; ++iheight)
	{
		byte* ppixel = image_data;
		for(size_t iwidth = 0; iwidth <src_rect.w; ++iwidth)
		{
			FIUC<color_t> uc((comp_t*)ppixel, alpha);
			mediator_type c(uc.r, uc.g, uc.b, uc.a);

			pixel_format_convertor::convert(
				ptex->get_pixel_format(),
				pixel_type_to_fmt<mediator_type>::fmt, 
				pdata,
				&c);

			ppixel += Bpp;
			pdata += color_infos[ptex->get_pixel_format()].size;
		}
		image_data += pitch;
	}

	ptex->unlock();
	return true;
}

h_texture create_texture_fi(
							renderer_impl* psr,
							const std::wstring& filename,
							pixel_format fmt, size_t destWidth, size_t destHeight, 
							const rect<size_t>& src
							)
{
	h_texture ret((texture*)NULL);
	rect<size_t> src_copy(src);

	string ansi_name;
	to_ansi_string(ansi_name, filename);

	FIBITMAP* image = load_image(ansi_name);
	if(image == NULL) return ret;
	if( ! check_image_type_support(image) ){
		return ret;
	}

	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(image);

	size_t bmp_width = FreeImage_GetWidth(image);
	size_t bmp_height = FreeImage_GetHeight(image);

	//设置缺省大小
	if((src.x | src.y | src.w | src.h) == 0){
		src_copy.w = bmp_width;
		src_copy.h = bmp_height;
	}
	if(destWidth == 0){destWidth = src_copy.w;}
	if(destHeight == 0){destHeight = src_copy.h;}

	//如果不是全局大小，则先copy / paste
	FIBITMAP* sub_image = NULL;
	if(src_copy.x == 0 && src_copy.y == 0 && src_copy.w == bmp_width && src_copy.h == bmp_height){
		sub_image = image;
	} else {
		sub_image = FreeImage_Copy(
			image, (int)src_copy.x, (int)src_copy.y, (int)(src_copy.x+src_copy.w), int(src_copy.y+src_copy.h)
			);
		FreeImage_Unload(image);
		if(!sub_image) return ret;

		src_copy.x = src_copy.y = 0;
	}

	//如果比例不是1：1,则scale
	FIBITMAP* scaled_image = NULL;
	if(src_copy.w == destWidth && src_copy.h == destHeight){
		scaled_image = sub_image;
	} else {
		scaled_image = FreeImage_Rescale(sub_image, int(destWidth), int(destHeight), FILTER_BILINEAR);
		FreeImage_Unload(sub_image);
		if(!scaled_image) return ret;
		src_copy.w = destWidth;
		src_copy.h = destHeight;
	}

	//设置源类型
	size_t src_width = FreeImage_GetWidth(scaled_image);
	size_t src_height = FreeImage_GetHeight(scaled_image);

	ret = psr->get_tex_mgr()->create_texture_2d(src_width, src_height, fmt);

	if(image_type == FIT_RGBAF){
		if(! copy_image_to_texture_rgba<color_rgba32f, FIRGBAF, float>(
			ret.get(), 0, scaled_image, src_copy)){
				ret.reset();
		}
	}
	if(image_type == FIT_BITMAP)
	{
		if(FreeImage_GetColorType(scaled_image) == FIC_RGBALPHA){
			if(! copy_image_to_texture_rgba<color_rgba8, RGBQUAD, BYTE>(
				ret.get(), 0, scaled_image, src_copy)){
					ret.reset();
			}
		} else {
			if(! copy_image_to_texture_rgba<color_rgba8, RGBTRIPLE, BYTE>(
				ret.get(), 0, scaled_image, src_copy, 255)){
					ret.reset();
			}
		}
	}

	FreeImage_Unload(scaled_image);
	return ret;
}

h_texture create_cube_texture_fi(
								 renderer_impl* psr,
								 const std::vector<std::wstring>& filenames,
								 pixel_format fmt
								 )
{
	h_texture ret;

	rect<size_t> src(0, 0, 0, 0);
	rect<size_t> src_copy(src);

	size_t destWidth(0);
	size_t destHeight(0);

	for(size_t i = 0; i < 6; ++i)
	{
		string ansi_name;
		to_ansi_string(ansi_name, filenames[i]);

		FIBITMAP* image = load_image(ansi_name);
		if(image == NULL) return ret;
		if( ! check_image_type_support(image) ){
			return ret;
		}

		FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(image);

		size_t bmp_width = FreeImage_GetWidth(image);
		size_t bmp_height = FreeImage_GetHeight(image);

		//设置缺省大小
		if((src.x | src.y | src.w | src.h) == 0){
			src_copy.w = bmp_width;
			src_copy.h = bmp_height;
		}
		if(destWidth == 0){destWidth = src_copy.w;}
		if(destHeight == 0){destHeight = src_copy.h;}

		//如果不是全局大小，则先copy / paste
		FIBITMAP* sub_image = NULL;
		if(src_copy.x == 0 && src_copy.y == 0 && src_copy.w == bmp_width && src_copy.h == bmp_height){
			sub_image = image;
		} else {
			sub_image = FreeImage_Copy(
				image, (int)src_copy.x, (int)src_copy.y, (int)(src_copy.x+src_copy.w), int(src_copy.y+src_copy.h)
				);
			FreeImage_Unload(image);
			if(!sub_image) return ret;

			src_copy.x = src_copy.y = 0;
		}

		//如果比例不是1：1,则scale
		FIBITMAP* scaled_image = NULL;
		if(src_copy.w == destWidth && src_copy.h == destHeight){
			scaled_image = sub_image;
		} else {
			scaled_image = FreeImage_Rescale(sub_image, int(destWidth), int(destHeight), FILTER_BILINEAR);
			FreeImage_Unload(sub_image);
			if(!scaled_image) return ret;
			src_copy.w = destWidth;
			src_copy.h = destHeight;
		}

		//设置源类型
		size_t src_width = FreeImage_GetWidth(scaled_image);
		size_t src_height = FreeImage_GetHeight(scaled_image);
		//size_t src_pitch = FreeImage_GetPitch(scaled_image);

		if(!ret){
			ret = psr->get_tex_mgr()->create_texture_cube(src_width, src_height, fmt);
		}
		texture_2d* ptex = (texture_2d*)&(((texture_cube*)(ret.get()))->get_face(cubemap_faces(i)));

		if(image_type == FIT_RGBAF){
			if(! copy_image_to_texture_rgba<color_rgba32f, FIRGBAF, float>(
				ptex, 0, scaled_image, src_copy)){
					ret.reset();
			}
		}
		if(image_type == FIT_BITMAP)
		{
			if(FreeImage_GetColorType(scaled_image) == FIC_RGBALPHA){
				if(! copy_image_to_texture_rgba<color_rgba8, RGBQUAD, BYTE>(
					ptex, 0, scaled_image, src_copy)){
						ret.reset();
				}
			} else {
				if(! copy_image_to_texture_rgba<color_rgba8, RGBTRIPLE, BYTE>(
					ptex, 0, scaled_image, src_copy, 255)){
						ret.reset();
				}
			}
		}

		FreeImage_Unload(scaled_image);
	}
	return ret;
}

h_texture create_cube_test_texture_fi(
									  renderer_impl* psr,
									  pixel_format /*fmt*/
									  )
{
	
	h_texture ret = psr->get_tex_mgr()->create_texture_cube(256, 256, pixel_format_color_bgra8);
	texture_cube* pcube = (texture_cube*)(ret.get());

	for(size_t i = 0; i < 6; ++i){
		surface* psurf = &pcube->get_surface(0, cubemap_faces(i));
		for(size_t j = 0; j < 256; ++j){
			for(size_t k = 0; k < 256; ++k){
				psurf->set_texel(j, k, color_bgra8(255, 255, 255, 255).to_rgba32f());
			}
		}
	}

	return ret;
}

void save_surface_to_file_fi(surface& surf, const _tstring& filename, pixel_format format)
{
	FREE_IMAGE_TYPE fit = FIT_UNKNOWN;
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	FIBITMAP* image = NULL;

	switch(format){
		case pixel_format_color_bgra8:
			fit = FIT_BITMAP;
			fif = FIF_PNG;
			image = FreeImage_AllocateT(fit, int(surf.get_width()), int(surf.get_height()), 32, 0x0000FF, 0x00FF00, 0xFF0000);
			break;
		case pixel_format_color_rgb32f:
			fit = FIT_RGBF;
			fif = FIF_HDR;
			image = FreeImage_AllocateT(fit, int(surf.get_width()), int(surf.get_height()), 96);
			break;
		default:
			custom_assert(false, "暂不支持该格式！");
			return;
	}

	byte* psurfdata = NULL;
	surf.lock((void**)&psurfdata, rect<size_t>(0, 0, surf.get_width(), surf.get_height()), lock_read_only);

	byte* pimagedata = FreeImage_GetBits(image);

	for(size_t ih = 0; ih < surf.get_height(); ++ih){
		pixel_format_convertor::convert_array(
			format,
			surf.get_pixel_format(),
			pimagedata, psurfdata,
			int(surf.get_width())
			);
		psurfdata += color_infos[surf.get_pixel_format()].size * surf.get_width();
		pimagedata += FreeImage_GetPitch(image);
	}

	surf.unlock();

	FreeImage_Save(fif, image, to_ansi_string(filename).c_str());
	FreeImage_Unload(image);
}