#pragma once

#include <ntstatus.h>

#define NOMINMAX
#pragma warning(push)
#pragma warning(disable: 4005)
#include <windows.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4201)
#include <d3d10umddi.h>
#pragma warning(pop)

#include <new>

#include <list>
#include <boost/thread/mutex.hpp>

class kmd_adapter;
class kmd_device;
class kmd_context;

extern HMODULE g_dll;
extern boost::mutex g_km_mutex;
extern std::list<kmd_adapter*> g_kmd_adapters;
extern std::list<kmd_device*> g_kmd_devices;
extern std::list<kmd_context*> g_kmd_contexts;

