#ifndef SALVIAR_TEXTURE_H
#define SALVIAR_TEXTURE_H

#include <salviar/include/colors.h>
#include <salviar/include/decl.h>
#include <salviar/include/enums.h>
#include <salviar/include/surface.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

#include <salviar/include/salviar_forward.h>

BEGIN_NS_SALVIAR();

class texture
{
protected:
	pixel_format fmt_;
	size_t min_lod_;
	size_t max_lod_;

	static size_t calc_lod_limit(size_t x, size_t y = 1, size_t z = 1)
	{
		EFLIB_ASSERT(x > 0 && y > 0 && z > 0, "");

		int rv = 0;
		while (x > 0 || y > 0 || z > 0){
			x >>= 1;
			y >>= 1;
			z >>= 1;
			++rv;
		}

		return rv;
	}
public:
	texture():max_lod_(0), min_lod_(0){}

	virtual texture_type get_texture_type()const = 0;

	size_t get_min_lod() const{return min_lod_;}
	size_t get_max_lod() const{return max_lod_;}
	pixel_format get_pixel_format() const{return fmt_;}

	virtual void map(void** pData, size_t subresource, map_mode mm) = 0;
	virtual void unmap(size_t subresource) = 0;

	virtual surface& get_surface(size_t subresource) = 0;
	virtual const surface& get_surface(size_t subresource) const = 0;

	virtual size_t get_width(size_t subresource) const= 0;
	virtual size_t get_height(size_t subresource) const= 0;
	virtual size_t get_depth(size_t subresource) const= 0;
	virtual size_t get_num_samples(size_t subresource) const= 0;
	
	virtual void set_max_lod(size_t miplevel) = 0;
	virtual void set_min_lod(size_t miplevel) = 0;

	virtual void gen_mipmap(filter_type filter, bool auto_gen) = 0;
};

class texture_2d : public texture
{
	std::vector<surface> surfs_;

	size_t width_;
	size_t height_;
	size_t num_samples_;

public:
	texture_2d(size_t width, size_t height, size_t num_samples, pixel_format format);
	void reset(size_t width, size_t height, size_t num_samples, pixel_format format);

	virtual texture_type get_texture_type()const
	{
		return texture_type_2d;
	};

	virtual void gen_mipmap(filter_type filter, bool auto_gen);

	virtual void map(void** pData, size_t subresource, map_mode mm);
	virtual void unmap(size_t subresource);

	virtual surface& get_surface(size_t subresource);
	virtual const surface& get_surface(size_t subresource) const;

	virtual size_t get_width(size_t subresource) const;
	virtual size_t get_height(size_t subresource) const;
	virtual size_t get_depth(size_t subresource) const;
	virtual size_t get_num_samples(size_t subresource) const;
	
	virtual void set_max_lod(size_t miplevel);
	virtual void set_min_lod(size_t miplevel);
};

class texture_cube : public texture
{
	std::vector<texture_2d> subtexs_;

	size_t width_;
	size_t height_;
	size_t num_samples_;

public:
	texture_cube(size_t width, size_t height, size_t num_samples, pixel_format format);
	void reset(size_t width, size_t height, size_t num_samples, pixel_format format);

	virtual texture_type get_texture_type()const
	{
		return texture_type_cube;
	};
	virtual void gen_mipmap(filter_type filter, bool auto_gen);

	virtual void map(void** pData, size_t subresource, map_mode mm);
	virtual void unmap(size_t subresource);

	virtual surface& get_surface(size_t subresource);
	virtual const surface& get_surface(size_t subresource) const;

	virtual texture& get_face(cubemap_faces face);
	virtual const texture& get_face(cubemap_faces face) const;

	virtual size_t get_width(size_t subresource) const;
	virtual size_t get_height(size_t subresource) const;
	virtual size_t get_depth(size_t subresource) const;
	virtual size_t get_num_samples(size_t subresource) const;
	
	virtual void set_max_lod(size_t miplevel);
	virtual void set_min_lod(size_t miplevel);
};

END_NS_SALVIAR()

#endif