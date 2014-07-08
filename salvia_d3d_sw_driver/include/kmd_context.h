#pragma once

class kmd_context
{
public:
	NTSTATUS create(D3DKMT_CREATECONTEXT* cc, kmd_device* dev);

	kmd_device* device() const
	{
		return device_;
	}

private:
	D3DKMT_HANDLE context_;
	kmd_device* device_;
};
