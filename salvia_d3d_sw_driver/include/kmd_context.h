#pragma once

class kmd_context
{
public:
	NTSTATUS create(D3DKMT_CREATECONTEXT* cc);

private:
	D3DKMT_HANDLE context_;
};
