#pragma once

#include <d3d12.h>
#include "Tools/Viewptr.h"

class CommandQueue;

namespace GFX
{
	bool device_init(bool WARP_adapter);
	bool device_destroy();
	void device_flush_all();

	ViewPtr<ID3D12Device4> get_device() noexcept;

	ViewPtr<CommandQueue> get_direct_queue()  noexcept;
	ViewPtr<CommandQueue> get_compute_queue() noexcept;
	ViewPtr<CommandQueue> get_copy_queue()    noexcept;
}