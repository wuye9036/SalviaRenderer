/*
Copyright (C) 2007-2010 Minmin Gong, Ye Wu

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
#include <salviax/include/resource/texture/freeimage/tex_io_freeimage.h>

#ifdef SALVIAXFREEIMAGE_ENABLED

#include <salviax/include/utility/freeimage_utilities.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/resource_manager.h>
#include <FreeImage.h>
#include <tchar.h>
#include <boost/static_assert.hpp>
#include <algorithm>

#pragma comment(lib, "freeimage.lib")

using namespace eflib;
using namespace std;
using namespace salviar;
using namespace softartx::utility;

BEGIN_NS_SALVIAXRESOURCE()
//从FIBITMAP将图像拷贝至Surface中。
//	拷贝将分为以下几步：
//		首先将FIBITMAP中的像素颜色提领出来，获得FIBITMAP中的分量信息。
//		根据分量信息，构造一个RGBA色序的SoftArt的中间颜色，并将FIBITMAP像素调整字节序拷贝到中间颜色中
//		将SoftArt中间颜色变换成目标Surface的颜色格式，拷贝到目标Surface中。
//	模板参数：FIColorT，FIBITMAP的存储格式
template<typename FIColorT> bool copy_image_to_surface(
	salviar::surface& surf, const rect<size_t>& dest_rect,
	FIBITMAP* image, const rect<size_t>& src_rect,
	typename FIUC<FIColorT>::CompT default_alpha = (FIUC<FIColorT>::CompT)(0) 
	)
{
	if (image == NULL){
		return false;
	}

	if ( src_rect.h != dest_rect.h || src_rect.w != dest_rect.w ){
		return false;
	}

	byte* pdata = NULL;
	surf.map((void**)&pdata, map_write);
	if(!pdata) { return false;}

	pdata += (dest_rect.y * surf.get_width() + dest_rect.x) * color_infos[surf.get_pixel_format()].size;

	BYTE* image_data = FreeImage_GetBits(image);
	size_t pitch = FreeImage_GetPitch(image);
	size_t bpp = (FreeImage_GetBPP(image) >> 3);

	image_data += (pitch * src_rect.y) + bpp * src_rect.x;

	for(size_t iheight = 0; iheight < src_rect.h; ++iheight)
	{
		byte* ppixel = image_data;
		for(size_t iwidth = 0; iwidth < src_rect.w; ++iwidth)
		{
			FIUC<FIColorT> uc((typename FIUC<FIColorT>::CompT*)ppixel, default_alpha);
			typename softart_rgba_color_type<FIColorT>::type c(uc.r, uc.g, uc.b, uc.a);

			pixel_format_convertor::convert(
				surf.get_pixel_format(),
				softart_rgba_color_type<FIColorT>::fmt, 
				pdata + iwidth * color_infos[surf.get_pixel_format()].size,
				&c);

			ppixel += bpp;
		}
		pdata += surf.get_width() * color_infos[surf.get_pixel_format()].size;
		image_data += pitch;
	}

	surf.unmap();
	return true;
}
//将Image的局部拷贝到surface里的指定区域内。如果源区域和目标区域大小不同，则进行双线插值的缩放。
bool texture_io_fi::load( salviar::surface& surf, const rect<size_t>& dest_region, FIBITMAP* img, const rect<size_t>& src_region ){
	rect<size_t> scaled_img_region ;
	FIBITMAP* scaled_img = make_bitmap_copy(scaled_img_region, dest_region.w, dest_region.h, img, src_region);
	
	if ( scaled_img == NULL ){
		return false;
	}

	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType( scaled_img );

	bool is_success = true;

	if(image_type == FIT_RGBAF){
		if(! copy_image_to_surface<FIRGBAF>(surf, dest_region, scaled_img, scaled_img_region) ){
			is_success = false;
		}
	}
	if(image_type == FIT_BITMAP)
	{
		if(FreeImage_GetColorType(scaled_img) == FIC_RGBALPHA){
			if(! copy_image_to_surface<RGBQUAD>(surf, dest_region, scaled_img, scaled_img_region) ){
				is_success = false;
			}
		} else {
			if( !copy_image_to_surface<RGBTRIPLE>(surf, dest_region, scaled_img, scaled_img_region) ){
				is_success = false;
			}
		}
	}

	FreeImage_Unload(scaled_img);

	return is_success;
}
//根据图像创建纹理。
salviar::h_texture texture_io_fi::load(salviar::renderer* pr, const std::_tstring& filename, salviar::pixel_format tex_pxfmt){
	FIBITMAP* img = load_image( filename );
	
	size_t src_w = FreeImage_GetWidth(img);
	size_t src_h = FreeImage_GetHeight(img);

	return load(pr, img, rect<size_t>(0, 0, src_w, src_h), tex_pxfmt, src_w, src_h);
}

//选取图像的一部分创建纹理。
salviar::h_texture texture_io_fi::load(salviar::renderer* pr,
		FIBITMAP* img, const eflib::rect<size_t>& src,
		salviar::pixel_format tex_pxfmt, size_t dest_width, size_t dest_height)
{
	salviar::h_texture ret((texture*)NULL);
	ret = pr->create_tex2d(src.w, src.h, 1, tex_pxfmt);

	if( !load(ret->get_surface(0), rect<size_t>(0, 0, dest_width, dest_height), img, src) ){
		ret.reset();
	}
	return ret;
}

//使用六张图像创建Cube纹理。Cube纹理的每面大小和第一张纹理的大小相同。如果其他文件的大小与第一张不同，则按第一张的大小缩放。
salviar::h_texture texture_io_fi::load_cube(salviar::renderer* pr, const vector<_tstring>& filenames, salviar::pixel_format fmt){
	salviar::h_texture ret;

	for(int i_cubeface = 0; i_cubeface < 6; ++i_cubeface){
		FIBITMAP* cube_img = load_image( filenames[i_cubeface] );
		if (cube_img == NULL){
			ret.reset();
			return ret;
		}

		//第一次调用的时候创建cube
		if ( !ret ){
			size_t img_w = FreeImage_GetWidth(cube_img);
			size_t img_h = FreeImage_GetHeight(cube_img);

			ret = pr->create_texcube( img_w, img_h, 1, fmt );
		}

		texture_cube* ptexcube = (texture_cube*)(ret.get());
		texture& face_tex = ptexcube->get_face(cubemap_faces(i_cubeface));
		rect<size_t> copy_region(0, 0, face_tex.get_width(0), face_tex.get_height(0));
		load( face_tex.get_surface(0), copy_region, cube_img, copy_region );
	}

	return ret;
}

// Save surface as PNG or HRD formatted file.
void texture_io_fi::save(const salviar::surface& surf, const std::_tstring& filename, salviar::pixel_format pxfmt){
	FREE_IMAGE_TYPE fit = FIT_UNKNOWN;
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	FIBITMAP* image = NULL;

	switch(pxfmt){
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
	EFLIB_ASSERT(false, "Unsupport format was used！");
	return;
	}

	byte* psurfdata = NULL;
	surf.map((void**)&psurfdata, map_read);

	byte* pimagedata = FreeImage_GetBits(image);

	for(size_t ih = 0; ih < surf.get_height(); ++ih){
		pixel_format_convertor::convert_array(
			pxfmt,
			surf.get_pixel_format(),
			pimagedata, psurfdata,
			int(surf.get_width())
			);
		psurfdata += color_infos[surf.get_pixel_format()].size * surf.get_width();
		pimagedata += FreeImage_GetPitch(image);
	}

	surf.unmap();

	FreeImage_Save(fif, image, to_ansi_string(filename).c_str());
	FreeImage_Unload(image);
}
END_NS_SALVIAXRESOURCE()

#endif