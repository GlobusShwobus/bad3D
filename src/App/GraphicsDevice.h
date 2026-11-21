#pragma once

#include <d3d12.h>
#include "Tools/Viewptr.h"

class CommandQueue;

namespace GFX
{
	// Initialized the graphics device. Not thread safe. Should be called on the main thread.
	bool device_init(bool WARP_adapter);

	// Destroys the graphics device. Not thread safe. Should be called on the same thread as init.
	bool device_destroy();

	// Flushes all of the command queues in the graphics device. Not thread safe.
	void device_flush_all();

	ViewPtr<ID3D12Device4> get_device() noexcept;

	ViewPtr<CommandQueue> get_direct_queue()  noexcept;
	ViewPtr<CommandQueue> get_compute_queue() noexcept;
	ViewPtr<CommandQueue> get_copy_queue()    noexcept;

	// A class that handles the RAII aspect of init/destroy of the device for better guarantees against throwing logic.
	// Not thread safe and should be called on the main thread.
	// Does not provide getters...
	class DeviceEasyRAII {
	public:
		DeviceEasyRAII(bool WARP_adapter) {
			device_init(WARP_adapter);
		}
		~DeviceEasyRAII() {
			device_destroy();
		}

		DeviceEasyRAII(const DeviceEasyRAII&) = delete;
		DeviceEasyRAII& operator=(const DeviceEasyRAII&) = delete;
		DeviceEasyRAII(DeviceEasyRAII&&)noexcept = delete;
		DeviceEasyRAII& operator=(DeviceEasyRAII&&)noexcept = delete;
	};
}