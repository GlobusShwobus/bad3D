#pragma once

#include "badDirectX.h"
#include <wrl/client.h>

struct CommandList
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>         command_allocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>     command_list      = nullptr;
};
