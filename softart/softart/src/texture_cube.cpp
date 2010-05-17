#include "../include/surface.h"
#include "../include/texture.h"
BEGIN_NS_SOFTART()

using namespace efl;

texture_cube::texture_cube(size_t width, size_t height, pixel_format format):subtexs_(6, texture_2d(0, 0, format)){
	for(size_t i = 0; i < 6; ++i){
		subtexs_[i].reset(width, height, format);
	}
}

void texture_cube::reset(size_t width, size_t height, pixel_format format){
	new (this) texture_cube(width, height, format);
}

void texture_cube::gen_mipmap(filter_type filter){
	for(size_t i = 0; i < 6; ++i){
		subtexs_[i].gen_mipmap(filter);
	}
}

void texture_cube::map(void** pData, size_t miplevel, map_mode mm, size_t z_slice){
	custom_assert(! is_mapped_, "");
	if(is_mapped_){
		*pData = NULL;
		return;
	}

	subtexs_[z_slice].map(pData, miplevel, mm);
	is_mapped_ = true;
	mapped_texture_ = z_slice;
}

void texture_cube::unmap(){
	custom_assert(is_mapped_, "");
	if(!is_mapped_){
		return;
	}

	subtexs_[mapped_texture_].unmap();
}

surface& texture_cube::get_surface(size_t miplevel, size_t z_slice){
	return subtexs_[z_slice].get_surface(miplevel);
}
const surface& texture_cube::get_surface(size_t miplevel, size_t z_slice) const{
	return subtexs_[z_slice].get_surface(miplevel);
}

surface& texture_cube::get_surface(size_t miplevel, cubemap_faces face){
	return subtexs_[face].get_surface(miplevel);
}
const surface& texture_cube::get_surface(size_t miplevel, cubemap_faces face) const{
	return subtexs_[face].get_surface(miplevel);
}

texture& texture_cube::get_face(cubemap_faces face){
	return subtexs_[face];
}

const texture& texture_cube::get_face(cubemap_faces face) const{
	return subtexs_[face];
}

size_t texture_cube::get_width(size_t mipmap) const{
	return subtexs_[0].get_width(mipmap);
}
size_t texture_cube::get_height(size_t mipmap) const{
	return subtexs_[0].get_height(mipmap);
}
size_t texture_cube::get_depth(size_t /*mipmap*/) const{
	return 6;
}

void texture_cube::set_max_lod(size_t miplevel){
	for(size_t i = 0; i < 6; ++i){
		subtexs_[i].set_max_lod(miplevel);
	}
}

void texture_cube::set_min_lod(size_t miplevel){
	for(size_t i = 0; i < 6; ++i){
		subtexs_[i].set_min_lod(miplevel);
	}
}
END_NS_SOFTART()
