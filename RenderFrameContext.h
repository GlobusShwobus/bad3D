#pragma once

#include "ObserverPtr.h"
#include "GPU_CORE.h"

struct RenderFrameContext
{
	ObserverPtr<ID3D12GraphicsCommandList> command_list;
	D3D12_CPU_DESCRIPTOR_HANDLE            resource_desc;
};