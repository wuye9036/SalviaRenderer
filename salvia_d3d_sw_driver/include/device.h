#pragma once

class umd_device
{
public:
    umd_device(umd_adapter* adapter, const D3D10DDIARG_CREATEDEVICE* args);

    // TODO

private:
    umd_adapter* adapter_;
};
