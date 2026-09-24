#pragma once

#include <memory>

#include <d3d12.h>
#include <wrl/client.h>

#include "App/CommandQueue.h"

class GraphicsDevice
{
public:
	GraphicsDevice();
	GraphicsDevice(bool use_warp_adapter);
	~GraphicsDevice() = default;

	GraphicsDevice(const GraphicsDevice&) = delete;
	GraphicsDevice& operator=(const GraphicsDevice&) = delete;
	GraphicsDevice(GraphicsDevice&&) = delete;
	GraphicsDevice& operator=(GraphicsDevice&&) = delete;

	ID3D12Device4* get_device() const noexcept{ return mDevice.Get(); }
	CommandQueue*  get_direct_queue()  noexcept { return mDirect.get(); }
	CommandQueue*  get_compute_queue() noexcept { return mCompute.get(); }
	CommandQueue*  get_copy_queue()    noexcept { return mCopy.get(); }

	void flush_all();

private:

	Microsoft::WRL::ComPtr<ID3D12Device4> mDevice;
	std::unique_ptr<CommandQueue>         mDirect;
	std::unique_ptr<CommandQueue>         mCompute;
	std::unique_ptr<CommandQueue>         mCopy;
};