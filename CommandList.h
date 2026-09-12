#pragma once

#include "badDirectX.h"
#include "EasyDirectX.h"
#include "ViewPtr.h"
#include <wrl/client.h>

struct CommandList
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>         command_allocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>     command_list      = nullptr;

	inline void transition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) const
	{
		assert(resource && "nullptr");
		D3D12_RESOURCE_BARRIER barrier = RESOURCE_BARRIER::transition(resource, before, after);
		command_list->ResourceBarrier(1, &barrier);
	}

	inline void clear_RTV(D3D12_CPU_DESCRIPTOR_HANDLE desc, FLOAT* clear_color) const
	{
		command_list->ClearRenderTargetView(desc, clear_color, 0, nullptr);
	}

	inline void clear_DSV(D3D12_CPU_DESCRIPTOR_HANDLE desc, FLOAT depth) const
	{
		command_list->ClearDepthStencilView(desc, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
	}
};
