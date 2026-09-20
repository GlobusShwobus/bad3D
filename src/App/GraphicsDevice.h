#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "D3D12/CommandQueue.h"

class GraphicsDevice
{
public:
	GraphicsDevice();
	~GraphicsDevice()
	{
		flush_all();
	}
	ID3D12Device4* device() const { return mDevice.Get(); }
	CommandQueue& direct() { return mDirect; }
	CommandQueue& compute() { return mCompute; }
	CommandQueue& copy() { return mCopy; }

	bool is_init() const noexcept { return mInitialised; }

	void flush_all();

private:

	Microsoft::WRL::ComPtr<ID3D12Device4> mDevice;
	CommandQueue mDirect;
	CommandQueue mCompute;
	CommandQueue mCopy;
	bool mInitialised = false;
};