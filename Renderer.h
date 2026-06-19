#ifndef RENDERER_H
#define RENDERER_H
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <cstdint>
#include <cassert>
#include <chrono>

#include "Window.h"
#include "MemeUtils.h"
#include "IRenderWindowSync.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

#define NUMBER_OF_BUFFERS 3

// adapter is the root, which creates devices and swapchains. swapcahins also need commandqueues
Microsoft::WRL::ComPtr<IDXGIAdapter4> get_adapter(bool useWarp)
{
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	UINT createFactoryFlags = 0;

#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	HRESULT hr = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
		throw std::runtime_error("failed to CreateDXGIFactory2");

	// if warp is used i need to use EnumWarpAdapter which takes Adapter1 as param but need to return Adapter4
	// so must do bullshit to get it
	Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter1;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter4;

	if (useWarp)
	{
		hr = dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1));

		if (FAILED(hr))
			throw std::runtime_error("failed to EnumWarpAdapter");

		hr = dxgiAdapter1.As(&dxgiAdapter4);

		if (FAILED(hr))
			throw std::runtime_error("failed to reinterpret adapter 1 to 4");
	}
	else
	{
		SIZE_T maxDedicatedVideoMemory = 0;

		for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);
			// Check to see if the adapter can create a D3D12 device without actually 
			// creating it. The adapter with the largest dedicated video memory
			// is favored.
			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && 
				SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
				dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;

				hr = dxgiAdapter1.As(&dxgiAdapter4);

				if (FAILED(hr))
					throw std::runtime_error("no warp: failed to reinterpret adapter 1 to 4");
			}

		}
	}

	return dxgiAdapter4;
}

// once the factory as created an adapater, the adapter is used to create a device. the device is the big creator type
Microsoft::WRL::ComPtr<ID3D12Device2> create_device(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter)
{
	Microsoft::WRL::ComPtr<ID3D12Device2> d3d12Device2;

	HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2));

	if (FAILED(hr))
		throw std::runtime_error("Failed to create ID3D12Device2");

#if defined(_DEBUG)
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> pInfoQueue;

	if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
	{
		// set break points on these 3 fuck-up-types
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// some specific messages may be supressed, to do that specify them with D3D12_MESSAGE_ and push them to the infoQueue filter

		// Suppress whole categories of messages
		// D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY severities[] = {
			D3D12_MESSAGE_SEVERITY_INFO 
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID deny_ids[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
		};

		D3D12_INFO_QUEUE_FILTER new_filter = {};
		//new_filter.DenyList.NumCategories = _countof(Categories);
		//new_filter.DenyList.pCategoryList = Categories;
		new_filter.DenyList.NumSeverities = _countof(severities);
		new_filter.DenyList.pSeverityList = severities;
		new_filter.DenyList.NumIDs = _countof(deny_ids);
		new_filter.DenyList.pIDList = deny_ids;

		hr = pInfoQueue->PushStorageFilter(&new_filter);

		if (FAILED(hr))
			throw std::runtime_error("idk, failed to set new debug layer filters");
	}
#endif
	return d3d12Device2;
}

// the commandqueue is created using the device
Microsoft::WRL::ComPtr<ID3D12CommandQueue> create_command_queue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT)
{
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue;

	D3D12_COMMAND_QUEUE_DESC desc = {};

	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	HRESULT hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&command_queue));

	if (FAILED(hr))
		throw std::runtime_error("failed to make command queue");

	return command_queue;
}

bool check_tearing_support()
{
	BOOL allowTearing = FALSE;

	// THIS MIGHT NOT BE TRUE ANYMORE. PROBABLY NOT EVEN!

	// Rather than create the DXGI 1.5 factory interface directly, we create the
	// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
	// graphics debugging tools which will not support the 1.5 factory interface 
	// until a future update.

	//Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;
	//
	//if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
	//{
	//	Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;
	//	if (SUCCEEDED(factory4.As(&factory5)))
	//	{
	//		if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
	//		{
	//			allowTearing = FALSE;
	//		}
	//	}
	//}

	Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;

	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory5))))
	{
		if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
			allowTearing = FALSE;
	}

	return allowTearing == TRUE;
}

// the swapchain is created using the command queue
Microsoft::WRL::ComPtr<IDXGISwapChain4> create_swap_chain(HWND hWnd, Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount)
{
	Microsoft::WRL::ComPtr<IDXGISwapChain4> dxgiSwapChain4;
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory4;
	UINT createFactoryFlags = 0;

#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	HRESULT hr = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4));

	if (FAILED(hr))
		throw std::runtime_error("Failed create factory");

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	// DISCARD IS FOR GAMES AND SUCH WHERE OLD STALE RENDERED BUT NOT YET PRESENTED FRAMES MAY BE DISCARDED IF A NEWER FRAME IS ALREADY QUEUED
	// for cameras and such squential is prefered to save every frame, at the cost of latency
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; 
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	// It is recommended to always allow tearing if tearing support is available.
	swapChainDesc.Flags = check_tearing_support() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;

	throw_if_failed(
		dxgiFactory4->CreateSwapChainForHwnd(
			commandQueue.Get(),
			hWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1
		)
	);

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.

	throw_if_failed(
		dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER)
	);

	throw_if_failed(
		swapChain1.As(&dxgiSwapChain4)
	);

	return dxgiSwapChain4;
}

// descriptor heap is created using the device
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> create_descriptor_heap(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;

	throw_if_failed(
		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap))
	);

	return descriptorHeap;
}

// command allocator is created using the device
Microsoft::WRL::ComPtr<ID3D12CommandAllocator> create_command_allocator(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;

	throw_if_failed(
		device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator))
	);

	return commandAllocator;
}

// command lists are created using the device. command lists store commands which are on the command allocators. command lists need to be sent to command queue
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> create_command_list(Microsoft::WRL::ComPtr<ID3D12Device2> device, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type)
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	throw_if_failed(
		device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList))
	);

	throw_if_failed(
		commandList->Close()
	);

	return commandList;
}

/*
CPU thread calls WaitForSingleObject()
		|
		v
CPU is STALLED (blocked, sleeping) <---- OS puts thread to sleep
		|
		|        meanwhile, GPU keeps executing...
		|
		v
GPU writes fenceValue into fence
		|
		v
Windows event is fired, CPU wakes up
		|
		v
CPU continues (safe to reuse frame resources)

1. GetCompletedValue() — cheap check, maybe the GPU is already done, no stall needed
2. If not done yet, SetEventOnCompletion() registers a Windows event to fire when the GPU hits that fence value
3. WaitForSingleObject() is what actually stalls the CPU thread — it yields to the OS until that event fires

*/
// a fence is created using the device. a fence is a inbetween object between GPU and CPU communication. it's best if one command queue signals 1 fence but this fence may have many listeners on the CPU side (multiple threads: thread 1 listens for 3, thread 2 for 6 etc)
Microsoft::WRL::ComPtr<ID3D12Fence> create_fence(Microsoft::WRL::ComPtr<ID3D12Device2> device)
{
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;

	throw_if_failed(
		device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	return fence;
}

HANDLE create_event_handle()
{
	HANDLE faceEvent;
	faceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(faceEvent && "failed to create fence event");

	return faceEvent;
}

uint64_t signal_fence(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue, Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue)
{
	uint64_t fenceValueForSignal = ++fenceValue;
	throw_if_failed(
		commandQueue->Signal(fence.Get(), fenceValueForSignal)
	);
	return fenceValueForSignal;
}

void wait_for_fence_value(Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration = std::chrono::milliseconds::max())
{
	if (fence->GetCompletedValue() < fenceValue)
	{
		throw_if_failed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
		::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
	}
}

class Renderer : public InterfaceRenderWindowSync
{
public:
	Renderer(Window& window):m_window(window)
	{
		m_window.sync_to_renderer_interface(this);
		m_TearingSupported = check_tearing_support();
		
		auto adapter = get_adapter(s_useWarp);
		
		m_device = create_device(adapter);

		m_commandQueue = create_command_queue(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

		m_swapChain = create_swap_chain(window.window(), m_commandQueue, window.get_width(), window.get_height(), s_NumFrames);

		m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

		m_RTVDescriptorHeap = create_descriptor_heap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, s_NumFrames);

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		update_RTV();

		for (int i = 0; i < s_NumFrames; ++i)
		{
			m_commandAllocators[i] = create_command_allocator(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		}

		m_commandList = create_command_list(m_device, m_commandAllocators[m_currentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);
	
		m_fence = create_fence(m_device);
		m_fenceEvent = create_event_handle();

		m_isInit = true;
	}
	~Renderer()override
	{
		m_window.sync_to_renderer_interface(nullptr);

		// Make sure the command queue has finished all commands before closing.
		flush_GPU();
		::CloseHandle(m_fenceEvent);
	}
	// DONT FREE MEMORY THE GPU IS STILL READING
// USE FLUSH BEFORE RESIZING THE SWAP CHAIN AND BEFORE SHUTDOWN
// MEANING LET THE GPU FINISH WHATEVER IT IS DOING BEFORE DOING MEMES
	void flush_GPU()
	{
		uint64_t fenceValueForSignal = signal_fence(m_commandQueue, m_fence, m_FenceValue);
		wait_for_fence_value(m_fence, m_FenceValue, m_fenceEvent);
	}
	bool is_init()const { return m_isInit; }

	void render()
	{
		// Before any commands can be recorded into the command list, 
		// the command allocator and command list needs to be reset to its initial state.
		auto commandAllocator = m_commandAllocators[m_currentBackBufferIndex];
		auto backBuffer = m_backBuffers[m_currentBackBufferIndex];

		commandAllocator->Reset();
		m_commandList->Reset(commandAllocator.Get(), nullptr);

		// clear the render target
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = backBuffer.Get();
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			m_commandList->ResourceBarrier(1, &barrier);

			FLOAT clearColor[] = { 0.4f,0.6f,0.9f,1.0f }; // light blue?
			D3D12_CPU_DESCRIPTOR_HANDLE rtv = {};
			rtv.ptr += m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + (m_currentBackBufferIndex * m_rtvDescriptorSize);

			m_commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		}

		// present
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = backBuffer.Get();
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			m_commandList->ResourceBarrier(1, &barrier);

			throw_if_failed(m_commandList->Close());

			ID3D12CommandList* const commandLists[] = { m_commandList.Get() };

			m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

			UINT syncInterval = m_VSync ? 1 : 0;
			UINT presentFlags = m_TearingSupported && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
			throw_if_failed(m_swapChain->Present(syncInterval, presentFlags));

			m_frameFenceValues[m_currentBackBufferIndex] = signal_fence(m_commandQueue, m_fence, m_FenceValue);

			m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

			wait_for_fence_value(m_fence, m_frameFenceValues[m_currentBackBufferIndex], m_fenceEvent);
		}
	}

	void update_RTV()
	{
		auto rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		for (int i = 0; i < NUMBER_OF_BUFFERS; ++i)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;

			throw_if_failed(
				m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer))
			);

			m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

			m_backBuffers[i] = backBuffer;

			rtvHandle.ptr += rtvDescriptorSize;
		}
	}

	void begin_frame();
	void end_frame();

protected:

	void on_resize(uint32_t width, uint32_t height) override
	{
		// Flush the GPU queue to make sure the swap chain's back buffers
		// are not being referenced by an in-flight command list.
		flush_GPU();

		for (int i = 0; i < s_NumFrames; ++i)
		{
			// Any references to the back buffers must be released
			// before the swap chain can be resized.
			m_backBuffers[i].Reset();
			m_frameFenceValues[i] = m_frameFenceValues[m_currentBackBufferIndex];
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		throw_if_failed(
			m_swapChain->GetDesc(&swapChainDesc)
		);
		throw_if_failed(
			m_swapChain->ResizeBuffers(s_NumFrames, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags)
		);

		m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

		update_RTV();
	}

private:
	static constexpr uint8_t s_NumFrames = NUMBER_OF_BUFFERS; // the number of back buffer surfaces for the swap chain. may not be less than 2
	static constexpr bool s_useWarp = false;  // controls whether to use software rasterizer (Windows Advanced Rasterization Platform), provides high end API basically
	bool m_isInit = false;                    // is set to true after DirectX12 shit is init

	Microsoft::WRL::ComPtr<ID3D12Device2>             m_device;       // DirectX device. basically is the main manager type. mostly creates objects
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>        m_commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4>           m_swapChain;    // The swap chain is responsible for presenting the rendered image to the window.
	Microsoft::WRL::ComPtr<ID3D12Resource>            m_backBuffers[s_NumFrames]; // Textures that are presented to the screen. They are transitioned between PRESENT and RENDER_TARGET states during rendering.

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;                    // records commands (draws, barriers, clears, copies... ) that are sent to the GPU . 1 list per CPU thread, reused per frame via reset().
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    m_commandAllocators[s_NumFrames]; // memory arena for recording the GPU commands into a command list. do not clear untill GPU is done with commands from the list (CPU frame and GPU are async)
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>      m_RTVDescriptorHeap;              // metadata that tells the GPU how to access resources (like back buffers)

	UINT m_rtvDescriptorSize = 0;  // vendor specific (NVidia / AMD may store descriptors differently). used as an offset. basically pointer arithmetic
	UINT m_currentBackBufferIndex = 0;  // current back buffered rendered

	// used for GPU synchronization
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence; // used to sync commands issjed to the command queue
	uint64_t m_FenceValue = 0;                   // the next fence value to signal the command queue
	uint64_t m_frameFenceValues[s_NumFrames] = {}; // For each rendered frame that could be “in-flight” on the command queue, the fence value that was used to signal the command queue needs to be tracked to guarantee that any resources that are still being referenced by the command queue are not overwritten.
	HANDLE m_fenceEvent; // handle to an OS event object that will be used to receive the notification that the fence has reached a specific value.

	// used to control the swap chains present method
	bool m_VSync = true;
	bool m_TearingSupported = false;

	Window& m_window;
};
#endif