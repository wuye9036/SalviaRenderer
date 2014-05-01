#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/colors.h>
#include <salviar/include/decl.h>
#include <salviar/include/enums.h>
#include <salviar/include/surface.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SALVIAR();

EFLIB_DECLARE_CLASS_SHARED_PTR(texture_2d);

class texture
{
protected:
	pixel_format    fmt_;
	size_t          min_lod_;
	size_t          max_lod_;

	static size_t calc_lod_limit(size_t x, size_t y = 1, size_t z = 1)
	{
		assert(x > 0 && y > 0 && z > 0);

		int rv = 0;
		while (x > 0 || y > 0 || z > 0)
		{
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

	size_t          get_min_lod() const{return min_lod_;}
	size_t          get_max_lod() const{return max_lod_;}
	pixel_format    get_pixel_format() const{return fmt_;}

	virtual surface_ptr const& get_surface(size_t subresource) const = 0;
	
	virtual size_t		get_width(size_t subresource)    const = 0;
	virtual size_t		get_height(size_t subresource)   const = 0;
	virtual size_t		get_depth(size_t subresource)    const = 0;
	virtual eflib::int4 get_int_size(size_t subresource) const = 0;
	virtual size_t		sample_count(size_t subresource) const = 0;
	
	virtual void set_max_lod(size_t miplevel) = 0;
	virtual void set_min_lod(size_t miplevel) = 0;

	virtual void gen_mipmap(filter_type filter, bool auto_gen) = 0;
};

class texture_2d : public texture
{
	std::vector<surface_ptr>    surfs_;
	size_t                      width_;
	size_t                      height_;
	size_t                      num_samples_;

public:
	texture_2d(size_t width, size_t height, size_t num_samples, pixel_format format);

	virtual texture_type get_texture_type()const
	{
		return texture_type_2d;
	};

	virtual void gen_mipmap(filter_type filter, bool auto_gen);

	virtual surface_ptr const& get_surface(size_t subresource) const;

	virtual size_t		get_width(size_t subresource) const;
	virtual size_t		get_height(size_t subresource) const;
	virtual size_t		get_depth(size_t subresource) const;
	virtual eflib::int4 get_int_size(size_t subresource) const;
	virtual size_t		sample_count(size_t subresource) const;
	
	virtual void set_max_lod(size_t miplevel);
	virtual void set_min_lod(size_t miplevel);
};

class texture_cube : public texture
{
	std::vector<texture_2d_ptr> faces_;
	size_t                      width_;
	size_t                      height_;
	size_t                      num_samples_;

public:
	texture_cube(size_t width, size_t height, size_t num_samples, pixel_format format);

	virtual texture_type get_texture_type()const
	{
		return texture_type_cube;
	};
	virtual void gen_mipmap(filter_type filter, bool auto_gen);

	virtual surface_ptr     const& get_surface(size_t subresource) const;
	virtual texture_2d_ptr  const& get_face(cubemap_faces face) const;

	virtual size_t		get_width(size_t subresource) const;
	virtual size_t		get_height(size_t subresource) const;
	virtual size_t		get_depth(size_t subresource) const;
	virtual eflib::int4 get_int_size(size_t subresource) const;
	virtual size_t		sample_count(size_t subresource) const;
	
	virtual void set_max_lod(size_t miplevel);
	virtual void set_min_lod(size_t miplevel);
};

END_NS_SALVIAR();
