template <typename T>
T round_up(T v, T align)
{
	return (v + align - 1) / align * align;
}

class struct_layout
{
public:
	struct_layout();
	
	size_t add_member(size_t sz)
	{
		size_t offset = 0;
		if(total_size_ % 16 == 0)
		{
			offset = total_size_;
		}
		else if ( total_size_ + sz >= round_up(total_size_, 16) )
		{
			offset = round_up(total_size_, 16);
		}
		
		total_size_ = offset + sz;
	}
	
	size_t size() const
	{
		return round_up(total_size_, 16);
	}
	
private:
	size_t total_size_;
};

enum class reg_slots: uint32_t
{
	unknown = 0,
	uniforms,				// cb#, s#, t#
	varying,				// v#
	outputs,				// o#
	count
};

enum class reg_types
{
	none = 0,
	texture,
	sampler,
	uav,
	cbuffer,
	output,
	count
};

struct reg_name
{
	reg_slots	slot;
	uint32_t	buffer_index;
	uint32_t	reg_index;
	uint32_t	elem;
	
	reg_name(reg_slots slot, uint32_t buf, uint32_t reg_index, uint32_t elem)
		: slot(slot), buffer_index(buf), reg_index(reg_index), elem(elem)
	{
	}
	
	reg_name(reg_name const& rhs)
		: slot(rhs.slot), buffer_index(rhs.buffer_index), reg_index(rhs.reg_index), elem(rhs.elem)
	{
	}
	
	reg_name& operator = (reg_name const& rhs)
	{
		slot = rhs.slot;
		buffer_index = rhs.buffer_index;
		reg_index = rhs.reg_index;
		elem = rhs.elem;
		return *this;
	}
	
	reg_name advance(size_t distance) const
	{
		return reg_name(slot, buffer_index, reg_index+distance, 0);
	}
};
 
typedef size_t register_handle;
class rfile_sasl_mapping
{
public:
	void initialize(shader_profile prof);
	
	alloc_result alloc_reg(size_t sz, reg_name const& rname)
	{	
		reg_name reg_end;
		return alloc_reg(reg_end, rname, sz);
	}
	
	alloc_result alloc_reg(reg_slots slot, size_t buf, size_t sz, reg_name const& rname)
	{
		reg_name reg_end;
		auto rslt = alloc_reg(reg_end, rname, sz);
		if( rslt != alloc_result::ok ) { return rslt; }
		
		assign_semantic(rname, reg_end, sv);
	}
	
	alloc_result alloc_reg(size_t sz, std::string const& vname, reg_name const& rname)
	{
		auto const& reg_beg = rname;
		auto reg_end = rname.advance( round_up(sz, 16) / 16 );
		auto reg_range = interval<reg_name>::right_open(reg_beg, reg_end);
		
		if( used_regs_[slot].intersects(reg_range) )
		{
			return alloc_result::register_has_been_allocated;
		}
		
		lut_var_reg_[vname] = reg_range;
	}
	
	alloc_result alloc_reg(reg_slots slot, size_t buf, size_t sz, std::string const& vname)
	{
		pending_vars_[slot].push_back( std::make_tuple(vname, buf, sz) );
	}

	void update_variable_reg_usage()
	{
		for (uint32_t slot = static_cast<uint32_t>(reg_slots::unknown); slot < static_cast<uint32_t>(reg_slots::count); ++slot)
		{
			
		}
	}
	
	void assign_semantic(reg_name const& beg, reg_name const& end, semantic_value const& sv_beg)
	{
		lut_reg_sv_[beg] = sv_beg;
		
		for( uint32_t rindex = beg.reg_index+1; rindex <= end.reg_index; ++rindex)
		{
			auto rname = reg_name(beg.slot, beg.buffer_index, rindex, 0);
			lut_reg_sv_[reg_name] = sv.advance_index(rindex - sv_beg.reg_index);
		}
	}
	
	reg_name find_reg(reg_slot slot, size_t buffer_index, semantic_value const& sem) const
	{
	}
	
	boost::icl::interval<reg_name>
			 find_reg(reg_slot slot, size_t buffer_index, std::string    const& vname) const
	{
	}
	
	void update_reg_physical_pos();
	
	uint32_t physical_register_count(reg_slot slot);
	
	uint32_t physical_pos(reg_slot slot, reg_name const& rname)
	{
		
	}
	
private:
	alloc_result alloc_reg(reg_name& beg, reg_name& end, reg_slots slot, size_t buf, size_t sz)
	{
		auto& slot_used_regs = used_regs_[slot];
		beg = slot_used_regs.empty() ? reg_name(slot, buf, 0, 0) : boost::icl::last(slot_used_regs).next();
		return alloc_reg(end, beg, sz);
	}
	
	alloc_result alloc_reg(reg_name& end, reg_name const& beg, size_t sz)
	{
		end = reg_name.advance( align_up(sz, 16) / 16 );
		slot_used_regs_.add( icl::right_open(beg, end) );
		return alloc_result::ok;
	}
	
	boost::icl::interval_set<reg_name>			used_regs_[static_cast<uint32_t>(reg_slots::count)];
	
	std::vector< std::tuple<std::string /*var name*/, uint32_t /*buffer*/, uint32_t /*reg*/> >
												pending_vars_[static_cast<uint32_t>(reg_slots::count)];
	// Lookup table
	//		1. semantic - registers (1:N)	- X
	//		2. register - semantic  (1:1)
	//		3. variable - register  (1:1)
	//		4. register - variable  (1:1)
	std::map<reg_name, semantic_value>			lut_reg_sv_ [static_cast<uint32_t>(reg_slots::count)];
	std::map<reg_name, std::string>				lut_reg_var_[static_cast<uint32_t>(reg_slots::count)];
	std::map<std::string, interval<reg_name>>	lut_var_reg_[static_cast<uint32_t>(reg_slots::count)];
	
	std::map<reg_name, uint32_t>				lut_reg_phy_[static_cast<uint32_t>(reg_slots::count)]
};

class rfile_hlsl_bc_mapping
{
	
};
