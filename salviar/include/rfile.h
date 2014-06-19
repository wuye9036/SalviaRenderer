#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/shader.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/icl/interval_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/math/math.h>
#include <eflib/include/string/ustring.h>

BEGIN_NS_SALVIAR();

enum class rfile_categories: uint32_t
{
	unknown = 0,
	offset,					// A special category, which register will be reallocated to correct category.
	uniforms,				// cb#, s#, t#
	varying,				// v#
	outputs,				// o#
	count
};

uint32_t RFILE_CATEGORY_BUFFER_COUNTS[] = 
{
	0,
	0,
	16 + 16 + 2 + 2,	// CB(16) + ICB(16) + Samplers + Textures + Global + Params
	1,					// Varying
	1
};

struct reg_file_uid
{
	reg_file_uid()
		:cat(rfile_categories::unknown), index(0)
	{
	}
	
	reg_file_uid(rfile_categories cat, uint32_t index)
		:cat(cat), index(index)
	{
	}
	
	reg_file_uid(reg_file_uid const& rhs)
		:cat(rhs.cat), index(rhs.index)
	{
	}
	
	reg_file_uid& operator = (reg_file_uid const& rhs)
	{
		cat = rhs.cat;
		index = rhs.index;
		return *this;
	}
	
	bool operator == (reg_file_uid const& rhs) const
	{
		return cat == rhs.cat && index == rhs.index;
	}

	static reg_file_uid global()
	{
		return reg_file_uid(rfile_categories::uniforms, 34 + 0);
	}

	static reg_file_uid params()
	{
		return reg_file_uid(rfile_categories::uniforms, 34 + 1);
	}

	static reg_file_uid varyings()
	{
		return reg_file_uid(rfile_categories::varying, 0);
	}

	rfile_categories	cat;
	uint32_t			index;
};

struct reg_name
{
	reg_file_uid	rfile;
	uint32_t		reg_index;
	uint32_t		elem;
	
	reg_name(): reg_index(0), elem(0)
	{
	}
	
	reg_name(rfile_categories cat, uint32_t rfile_index, uint32_t reg_index, uint32_t elem)
		: rfile(cat, rfile_index), reg_index(reg_index), elem(elem)
	{
	}
	
	reg_name(reg_file_uid rfile, uint32_t reg_index, uint32_t elem)
		: rfile(rfile), reg_index(reg_index), elem(elem)
	{
	}
	
	reg_name(reg_name const& rhs)
		: rfile(rhs.rfile), reg_index(rhs.reg_index), elem(rhs.elem)
	{
	}
	
	reg_name& operator = (reg_name const& rhs)
	{
		rfile = rhs.rfile;
		reg_index = rhs.reg_index;
		elem = rhs.elem;
		return *this;
	}
	
	reg_name advance(size_t distance) const
	{
		return reg_name(rfile, static_cast<uint32_t>(reg_index + distance), 0);
	}
	
	bool valid() const
	{
		return rfile.cat == rfile_categories::unknown;
	}

	bool operator < (reg_name const& rhs) const
	{
		assert(rfile == rhs.rfile);
		if( reg_index < rhs.reg_index ) return true;
		if( reg_index > rhs.reg_index ) return false;
		return elem < rhs.elem;
	}
};

struct reg_file
{
public:
	virtual std::pair<reg_name, reg_name>
					 find_reg(eflib::fixed_string const& vname) const	= 0;
	virtual reg_name find_reg(semantic_value const& sv) const			= 0;
	virtual uint32_t used_reg_count() const								= 0;
	virtual uint32_t reg_addr(reg_name const& rname) const				= 0;
	virtual ~reg_file() {}
};

END_NS_SALVIAR();
