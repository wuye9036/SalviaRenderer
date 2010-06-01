#include <softartx/include/utility/freeimage_utilities.h>

#ifdef SOFTARTX_FREEIMAGE_ENABLED

using namespace std;
using namespace efl;
using namespace softart;

BEGIN_NS_SOFTARTX_UTILITY()

FIBITMAP* load_image(const std::_tstring& filename, int flag)
{
	string ansi_filename = to_ansi_string(filename);

	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(ansi_filename.c_str(), 0);
	if(fif == FIF_UNKNOWN){
		fif = FreeImage_GetFIFFromFilename(ansi_filename.c_str());
	}
	if( fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif) ){
		return FreeImage_Load(fif, ansi_filename.c_str(), flag);
	}

	return NULL;
}

bool check_image_type_support(FIBITMAP* image)
{
	if (NULL == image){ return false; }

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

FIBITMAP* make_bitmap_copy(
	rect<size_t>& out_region, size_t dest_width, size_t dest_height,
	FIBITMAP* image, const rect<size_t>& src_region
	)
{
	if(image == NULL || ! check_image_type_support(image) ) {
		return NULL;
	}

	//FREE_IMAGE_TYPE img_type = FreeImage_GetImageType(image);
	size_t img_w = FreeImage_GetWidth(image);
	size_t img_h = FreeImage_GetHeight(image);

	// 如果不需要缩放，则直接返回传入的Image及Region
	if ( dest_width == src_region.w && dest_height == src_region.h ){
		out_region = src_region;
		return image;
	}

	// 如果不是全局大小，则先拷贝一个副本
	FIBITMAP* sub_image = NULL;
	if(src_region.x == 0 && src_region.y == 0 && src_region.w == img_w && src_region.h == img_h){
		sub_image = image;
	} else {
		sub_image = FreeImage_Copy(
			image, (int)src_region.x, (int)src_region.y, (int)(src_region.x+src_region.w), int(src_region.y+src_region.h)
			);
		FreeImage_Unload(image);
		if(!sub_image) return NULL;
	}

	// scale
	FIBITMAP* scaled_image = NULL;
	scaled_image = FreeImage_Rescale(sub_image, int(dest_width), int(dest_height), FILTER_BILINEAR);
	FreeImage_Unload(sub_image);

	out_region = rect<size_t>(0, 0, dest_width, dest_height);

	return scaled_image;
}

END_NS_SOFTARTX_UTILITY()

#endif //SOFTARTX_FREEIMAGE_ENABLED