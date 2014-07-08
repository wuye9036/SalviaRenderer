#pragma once

class kmd_device
{
public:
	NTSTATUS create(D3DKMT_CREATEDEVICE* cd);

	NTSTATUS create_context(D3DKMT_CREATECONTEXT* cd);
	NTSTATUS escape(const D3DKMT_ESCAPE* esc);

	umd_device* get_umd_device() const
	{
		return umd_device_;
	}

private:
	D3DKMT_HANDLE device_;
	umd_device* umd_device_;
};
