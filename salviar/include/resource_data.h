#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/enums.h>

#include <eflib/include/memory/atomic.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/shared_array.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <vector>

BEGIN_NS_SALVIAR();

class aligned_array
{
public:
	aligned_array(size_t sz, size_t alignment);
	
	void	reallocate();

	void*	data() const;
	size_t	size() const;
	size_t	alignment() const;

private:
	boost::shared_array<char>	data_;
	char*						aligned_data_;
	size_t						size_;
	size_t						alignment_;
};

class renderer_impl;

class resource_data
{
private:
	renderer_impl*	renderer_;
	resource_usage	usage_;
	aligned_array	data_;
	bool			client_mapped_;

public:
	resource_data(
		renderer_impl* renderer,
		size_t sz, size_t alignment, resource_usage usage,
		void* init_data
		)
		:renderer_(renderer), data_(sz, alignment), usage_(usage), client_mapped_(false)
	{
		memcpy(data_.data(), init_data, sz);
	}

	size_t size() const
	{
		return data_.size();
	}

	bool check_mappable(map_mode mm, bool called_by_device)
	{
		if(client_mapped_)
		{
			return false;
		}
		
		if(called_by_device && (usage_ & resource_device) == 0)
		{
			return false;
		}
		
		if(!called_by_device && (usage_ & resource_client) == 0)
		{
			return false;
		}
		
		switch( mm )
		{
		case map_read:
			return (usage_ & resource_read) != 0;
		case map_read_write:
			return usage_ == (usage_ & resource_read) != 0 && (usage_ & resource_write) != 0;
		case map_write_no_overwrite:
		case map_write_discard:
		case map_write:
			return (usage_ & resource_write) != 0;
		}
		
		return false;
	}
	
	map_result map(void** out_data, map_mode mm, bool do_not_wait, bool force_sync)
	{
		if ( !check_mappable(mm, false) )
		{
			return map_failed;
		}
		
		if( mm == map_write_discard )
		{
			data_.reallocate();
		}
		else if ( mm != map_write_no_overwrite || force_sync )
		{
#if 0
			// TODO
			if( !renderer_->is_free() )
			{
				if(do_not_wait)
				{
					return map_do_not_wait;
				}
				renderer_->flush();
			}
#endif
		}

		client_mapped_ = true;
		*out_data = data_.data();
	}
	
	void unmap()
	{
		client_mapped_ = false;
	}
	
	map_result device_map(aligned_array& out_data, map_mode mm)
	{
		if( !check_mappable(mm, true) )
		{
			return map_failed;
		}
		
		out_data = data_;
		return map_succeed;
	}
	
	void device_unmap()
	{
	}

	// TODO Deprecated
	void* raw_data(size_t offset)
	{
		return reinterpret_cast<char*>( data_.data() ) + offset;
	}
};

END_NS_SALVIAR();