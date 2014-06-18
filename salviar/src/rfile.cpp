#if 0

#include <salviar/include/rfile.h>

BEGIN_NS_SALVIAR();

enum class reg_types: uint32_t
{
	none = 0,
	texture,
	sampler,
	uav,
	cbuffer,
	output,
	count
};

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
		else if ( total_size_ + sz >= eflib::round_up(total_size_, 16) )
		{
			offset = eflib::round_up(total_size_, 16);
		}
		
		total_size_ = offset + sz;
	}
	
	size_t size() const
	{
		return eflib::round_up(total_size_, 16);
	}
	
private:
	size_t total_size_;
};


class rfile_sasl_mapping
{
public:
	void initialize(shader_profile prof);
	
	rfile* buffer(rfile_categories cat, uint32_t buf_index)
	{
		auto& slot_bufs = buffer_rfile_[static_cast<uint32_t>(cat)];
		
		if( buf_index < slot_bufs.size() )
		{
			if(!slot_bufs[buf_index])
			{
				slot_bufs[buf_index].reset(new rfile(cat, buf_index));
			}
			return slot_bufs[buf_index].get();
		}
		else 
		{
			return nullptr;
		}
	}
	
	rfile* buffer(rfile_uid buf_uid)
	{
		return buffer(buf_uid.cat, buf_uid.index);
	}
	
	rfile const* buffer(rfile_categories cat, uint32_t buf_index) const
	{
		auto& slot_bufs = buffer_rfile_[static_cast<uint32_t>(cat)];
		
		if( buf_index < slot_bufs.size() )
		{
			return slot_bufs[buf_index].get();
		}
		else 
		{
			return nullptr;
		}
	}
	
	rfile const* buffer(rfile_uid buf_uid) const
	{
		return buffer(buf_uid.cat, buf_uid.index);
	}
	
	void update_reg_address()
	{
		for(uint32_t cat = static_cast<uint32_t>(rfile_categories::unknown); cat = static_cast<uint32_t>(rfile_categories::count); ++cat)
		{
			auto& slot_bufs		= buffer_rfile_[cat];
			auto& slot_buf_addr = buf_start_addr_[cat];
			
			uint32_t total = 0;
			for(auto& rf: slot_bufs)
			{
				slot_buf_addr.push_back(total);
				total += rf->used_reg_count();
			}
			used_reg_count_[cat] = total;
		}
	}
	
	uint32_t used_reg_count(rfile_categories cat)
	{
		return used_reg_count_[static_cast<uint32_t>(cat)];
	}
	
	uint32_t reg_addr(reg_name const& rname)
	{
		uint32_t cat = static_cast<uint32_t>(rname.rf.cat);
		return
			buf_start_addr_[cat][rname.rf.index] +
			buffer_rfile_[cat][rname.rf.index]->reg_addr(rname);
	}
	
private:
	std::vector< std::unique_ptr<rfile_uid> >
				buffer_rfile_	[static_cast<uint32_t>(rfile_categories::count)];
	uint32_t	used_reg_count_	[static_cast<uint32_t>(rfile_categories::count)];
	std::vector<uint32_t>
				buf_start_addr_	[static_cast<uint32_t>(rfile_categories::count)];				
};

END_NS_SALVIAR();

#endif
