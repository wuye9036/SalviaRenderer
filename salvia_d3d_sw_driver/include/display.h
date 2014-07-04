#pragma once

#include <vector>
#include <string>

class display
{
public:
	display();
	display(const std::wstring& name, uint32_t state);

	bool attach_owner(umd_device* owner);
	bool attach_owner_locked(umd_device* owner);
	void detach_owner(umd_device* owner);
	void detach_owner_locked(umd_device* owner);
	bool is_owner(umd_device* owner);

	const std::wstring& name() const
	{
		return name_;
	}
	uint32_t state_flags() const
	{
		return state_flags_;
	}
	HMONITOR monitor() const
	{
		return monitor_;
	}
	void monitor(HMONITOR mon)
	{
		monitor_ = mon;
	}

private:
	std::wstring name_;
	uint32_t state_flags_;
	HMONITOR monitor_;
	umd_device* owner_;
};

class display_mgr
{
public:
	display_mgr();

	static const boost::shared_ptr<display_mgr>& instance();
	static void destroy();
	static boost::mutex& display_mutex()
	{
		return mutex_;
	}

	void detach_owner(umd_device* owner);

	uint32_t num_displays() const
	{
		return static_cast<uint32_t>(displays_.size());
	}
	display* get_display(uint32_t index)
	{
		return (index < displays_.size()) ? &displays_[index] : nullptr;
	}

private:
	static BOOL CALLBACK monitor_enum_proc(HMONITOR monitor, HDC hdc, LPRECT rc, LPARAM param);

private:
	static boost::mutex mutex_;
	static boost::shared_ptr<display_mgr> disp_mgr_;

	std::vector<display> displays_;
};
