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
	pixel_format			 fmt_;
	size_t					 sample_count_;
	size_t					 min_lod_;
	size_t					 max_lod_;
	eflib::uint4             size_;
	std::vector<surface_ptr> surfs_;

	static size_t calc_lod_limit(eflib::uint4 sz)
	{
		size_t rv = 0;
		auto max_sz = std::max( sz[0], std::max(sz[1], sz[2]) );
		while (max_sz > 0)
		{
			max_sz >>= 1;
			++rv;
		}

		return rv;
	}

public:
	texture(): max_lod_(0), min_lod_(0)
	{
	}

	virtual texture_type get_texture_type() const = 0;

	pixel_format format() const
	{
		return fmt_;
	}
	
	size_t min_lod() const
	{
		return min_lod_;
	}

	size_t max_lod() const
	{
		return max_lod_;
	}
	
	surface_ptr subresource(size_t index) const
	{
		if(max_lod_ <= index && index <= index)
		{
			return surfs_[index]; 
		}
		return surface_ptr();
	}
	
	eflib::uint4 size() const
	{
		return size_;
	}

	eflib::uint4 size(size_t subresource_index) const
	{
		auto subres = subresource(subresource_index);
		if (subres)
		{
			return eflib::uint4(0, 0, 0, 0);
		}
		return subres->size();
	}

	size_t sample_count() const
	{
		return sample_count_;
	}
	
	void max_lod(int miplevel)
	{
		max_lod_ = miplevel;
	}

	void min_lod(int miplevel)
	{
		min_lod_ = miplevel;
	}

	virtual void gen_mipmap(filter_type filter, bool auto_gen) = 0;
};

class texture_2d : public texture
{
public:
	texture_2d(size_t width, size_t height, size_t num_samples, pixel_format format);

	virtual texture_type get_texture_type() const
	{
		return texture_type_2d;
	};

	virtual void gen_mipmap(filter_type filter, bool auto_gen);
};

class texture_cube : public texture
{
public:
	texture_cube(size_t width, size_t height, size_t num_samples, pixel_format format);

	virtual texture_type get_texture_type() const
	{
		return texture_type_cube;
	};

	surface_ptr subresource(size_t face, size_t lod) const
	{
		return texture::subresource(lod * 6 + face);
	}

	virtual void gen_mipmap(filter_type filter, bool auto_gen);
};

END_NS_SALVIAR();
