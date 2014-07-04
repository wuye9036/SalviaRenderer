#pragma once

class kmd_device
{
public:
	NTSTATUS create(D3DKMT_CREATEDEVICE* cd);

	NTSTATUS create_context(D3DKMT_CREATECONTEXT* cd);

private:
	D3DKMT_HANDLE device_;
};
