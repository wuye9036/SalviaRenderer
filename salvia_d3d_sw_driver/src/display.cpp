#include <salvia_d3d_sw_driver/include/common.h>

#include <eflib/include/platform/typedefs.h>

#include <boost/make_shared.hpp>

#include <salvia_d3d_sw_driver/include/display.h>

boost::mutex display_mgr::mutex_;
boost::shared_ptr<display_mgr> display_mgr::disp_mgr_;

display::display()
	: state_flags_(0), monitor_(nullptr), owner_(nullptr)
{
}

display::display(const std::wstring& name, uint32_t state)
	: name_(name), state_flags_(state), monitor_(nullptr), owner_(nullptr)
{
}

bool display::attach_owner(umd_device* owner)
{
	boost::mutex::scoped_lock lock(display_mgr::display_mutex());
	return this->attach_owner_locked(owner);
}

bool display::attach_owner_locked(umd_device* owner)
{
	if (owner_ != nullptr)
	{
		return false;
	}
	else
	{
		owner_ = owner;
		return true;
	}
}

void display::detach_owner(umd_device* owner)
{
	boost::mutex::scoped_lock lock(display_mgr::display_mutex());
	this->detach_owner_locked(owner);
}

void display::detach_owner_locked(umd_device* owner)
{
	if (this->is_owner(owner))
	{
		owner_ = nullptr;
	}
}

bool display::is_owner(umd_device* owner)
{
	return owner_ == owner;
}


display_mgr::display_mgr()
{
	for (uint32_t i = 0;; ++ i)
	{
		DISPLAY_DEVICEW dd;
		dd.cb = sizeof(dd);
		if (!EnumDisplayDevicesW(nullptr, i, &dd, 0))
		{
			break;
		}

		displays_.push_back(display(dd.DeviceName, dd.StateFlags));
	}

	EnumDisplayMonitors(nullptr, nullptr, monitor_enum_proc, reinterpret_cast<LPARAM>(this));
}

void display_mgr::destroy()
{
	boost::mutex::scoped_lock lock(mutex_);
	disp_mgr_.reset();
}

const boost::shared_ptr<display_mgr>& display_mgr::instance()
{
	if (!disp_mgr_)
	{
		boost::mutex::scoped_lock lock(mutex_);
		if (!disp_mgr_)
		{
			disp_mgr_ = boost::make_shared<display_mgr>();
		}
	}

	return disp_mgr_;
}

void display_mgr::detach_owner(umd_device* owner)
{
	boost::mutex::scoped_lock lock(mutex_);
	for (size_t i = 0; i < displays_.size(); ++ i)
	{
		displays_[i].detach_owner_locked(owner);
	}
}

BOOL CALLBACK display_mgr::monitor_enum_proc(HMONITOR mon, HDC hdc, LPRECT rc, LPARAM param)
{
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(rc);

	display_mgr* disp_mgr = reinterpret_cast<display_mgr*>(param);

	MONITORINFOEXW mi;
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfoW(mon, reinterpret_cast<MONITORINFO*>(&mi)))
	{
		return FALSE;
	}
	else
	{
		for (size_t i = 0; i < disp_mgr->displays_.size(); ++ i)
		{
			if (disp_mgr->displays_[i].name() == mi.szDevice)
			{
				disp_mgr->displays_[i].monitor(mon);
			}
		}

		return TRUE;
	}
}
