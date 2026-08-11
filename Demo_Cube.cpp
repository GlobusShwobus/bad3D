#include "Demo_Cube.h"

#include <d3dcompiler.h>
#pragma comment(lib, "D3Dcompiler_47.lib")
#include <algorithm>

#include "Utils.h"

// Vertex data for a colored cube.
struct VertexPosColor
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Color;
};

static VertexPosColor gCubeVerts[8] = {
	{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
	{ DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
	{ DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f),  DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
	{ DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f),  DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
	{ DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
	{ DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
	{ DirectX::XMFLOAT3(1.0f,  1.0f,  1.0f),  DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
	{ DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f),  DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
};

static WORD gCubeIndicies[36] =
{
	0, 1, 2, 0, 2, 3,
	4, 6, 5, 4, 7, 6,
	4, 5, 1, 4, 1, 0,
	3, 2, 6, 3, 6, 7,
	1, 5, 6, 1, 6, 2,
	4, 0, 3, 4, 3, 7
};


DemoCube::DemoCube()
{
	// check of directX math library support
	if (!DirectX::XMVerifyCPUSupport())
	{
		throw std::runtime_error("memes");
	}
}
DemoCube::~DemoCube()
{
	unload_content();
}

void DemoCube::load_content(Application& app)
{
	mDevice = app.get_device();
	mDireectCommandQueue = app.get_command_queue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto copy_command_queue = app.get_command_queue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto copy_command_list = copy_command_queue->get_command_list();
	mWindow = app.get_window();

	// scissor rect is responsible for culling any pixels that are not within the dimensions of the RT
	mScissorRect = D3D12_RECT{ 0,0,LONG_MAX, LONG_MAX };

	// viewport rect is responsible for saying where to write to but it should not be outside the RT
	UINT width, height;
	mWindow->get_client_size(width, height);
	mViewport = D3D12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };

	// represents the vertical vield of view of the camera (it looks like a cone but not really, it kind of scales shit instead)
	mFOV = 45.0f;

	// upload vertex buffer data and assign the view data
	D3D12Resource intermediateVertexBuffer; // a temporary, make sure to stall the CPU util copy command queue is donzo
	update_buffer_resource(
		copy_command_list,
		&mVertexBuffer,
		&intermediateVertexBuffer, 
		_countof(gCubeVerts),
		sizeof(VertexPosColor),
		gCubeVerts
	);

	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = sizeof(gCubeVerts);
	mVertexBufferView.StrideInBytes = sizeof(VertexPosColor);

	// upload index buffer and assign the view data
	D3D12Resource internmeduateIndexBuffer;
	update_buffer_resource(
		copy_command_list,
		&mIndexBuffer,
		&internmeduateIndexBuffer,
		_countof(gCubeIndicies),
		sizeof(WORD),
		gCubeIndicies
	);

	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	mIndexBufferView.SizeInBytes = sizeof(gCubeIndicies);


	// bullshit
	mSignalTracker.resize(mWindow->get_buffer_count(), 0);
	mTimer.reset();
}

void DemoCube::on_update()
{
	const float dt = mTimer.dt_float();

	mouse_resolve();
	kb_resolve();
	mouse.update_mouse_buttons(dt);
}

void DemoCube::on_render() 
{
	ObserverPtr<ID3D12GraphicsCommandList2> command_list = mDireectCommandQueue->get_command_list();
	ObserverPtr<ID3D12Resource> current_back_buffer = mWindow->get_buffer();
	D3D12_CPU_DESCRIPTOR_HANDLE buffer_desc = mWindow->get_buffer_desc();

	set_transition_barrier(command_list, current_back_buffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// any rendering logic goes here until another transition barrier
	clearRTV(command_list, buffer_desc, color);

	set_transition_barrier(command_list, current_back_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	command_list->Close();

	const UINT64 current_index = mWindow->get_buffer_index();
	const UINT64 signal_val = mDireectCommandQueue->execute();
	mSignalTracker[current_index] = signal_val;

	mWindow->present_to_display();

	const UINT64 some_new_buffer_index = mWindow->get_buffer_index();

	mDireectCommandQueue->wait(mSignalTracker[some_new_buffer_index]);
}

void DemoCube::on_resize() 
{
	UINT client_width, client_height;
	mWindow->get_client_size(client_width, client_height);

	const UINT buffer_width = mWindow->get_buffer_width();
	const UINT buffer_height = mWindow->get_buffer_height();

	if (buffer_width != client_width || buffer_height != client_height)
	{
		mDireectCommandQueue->flush();

		const UINT current_val = mSignalTracker[mWindow->get_buffer_index()];

		for (auto& fence_val : mSignalTracker)
			fence_val = current_val;

		mWindow->resize(client_width, client_height);
	}
}

void DemoCube::on_key_event(UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	kb.resolve_message(uMsg, wParam, lParam);
}

void DemoCube::on_mouse_event(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	mouse.resolve_message(uMsg, wParam, lParam);
}

void DemoCube::kb_resolve()
{
	static bool fullscreen = false;
	static bool f11_previous = false;

	const bool f11_current = kb.get_keys()[VK_F11];

	if (f11_current && !f11_previous)
	{
		fullscreen = !fullscreen;
		mWindow->toggle_fullscreen(fullscreen);
	}

	f11_previous = f11_current;
}

void DemoCube::mouse_resolve()
{
	if (mouse.get_button(MouseButtonType::Left).is_down())
	{
		color[0] = 1;
		color[1] = 0;
	}
	else
	{
		color[0] = 0;
		color[1] = 1;
	}
}

void DemoCube::set_transition_barrier(ObserverPtr<ID3D12GraphicsCommandList2> command_list, ObserverPtr<ID3D12Resource> back_buffer, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER barrier = {};

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = back_buffer.get();
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	command_list->ResourceBarrier(1, &barrier);
}

void DemoCube::clearRTV(ObserverPtr<ID3D12GraphicsCommandList2> command_list, D3D12_CPU_DESCRIPTOR_HANDLE desc, FLOAT* clear_color)
{
	command_list->ClearRenderTargetView(desc, clear_color, 0, nullptr);
}

void DemoCube::update_buffer_resource(ObserverPtr<ID3D12GraphicsCommandList2> command_list,
	ID3D12Resource** pDestinationResource,
	ID3D12Resource** pIntermediateResource,
	size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
	const std::size_t bufferSize = numElements * elementSize;

	D3D12_HEAP_PROPERTIES dest_heap_desc{};
	dest_heap_desc.Type = D3D12_HEAP_TYPE_DEFAULT;
	dest_heap_desc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	dest_heap_desc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	dest_heap_desc.CreationNodeMask = 1;
	dest_heap_desc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC dest_buffer_desc{};
	dest_buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	dest_buffer_desc.Alignment = 0;
	dest_buffer_desc.Width = bufferSize;
	dest_buffer_desc.Height = 1;
	dest_buffer_desc.DepthOrArraySize = 1;
	dest_buffer_desc.MipLevels = 1;
	dest_buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
	dest_buffer_desc.SampleDesc = { 1, 0 };
	dest_buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	dest_buffer_desc.Flags = flags;

	execute_and_test_hresult(
		mDevice->CreateCommittedResource(
			&dest_heap_desc,
			D3D12_HEAP_FLAG_NONE,
			&dest_buffer_desc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(pDestinationResource))
	);

	if (bufferData)
	{
		D3D12_HEAP_PROPERTIES intermediate_heap_desc{};
		intermediate_heap_desc.Type = D3D12_HEAP_TYPE_UPLOAD;
		intermediate_heap_desc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		intermediate_heap_desc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		intermediate_heap_desc.CreationNodeMask = 1;
		intermediate_heap_desc.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC intermediate_buffer_desc{};
		intermediate_buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		intermediate_buffer_desc.Alignment = 0;
		intermediate_buffer_desc.Width = bufferSize;
		intermediate_buffer_desc.Height = 1;
		intermediate_buffer_desc.DepthOrArraySize = 1;
		intermediate_buffer_desc.MipLevels = 1;
		intermediate_buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
		intermediate_buffer_desc.SampleDesc = { 1, 0 };
		intermediate_buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		intermediate_buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		execute_and_test_hresult(
			mDevice->CreateCommittedResource(
				&intermediate_heap_desc,
				D3D12_HEAP_FLAG_NONE,
				&intermediate_buffer_desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(pIntermediateResource))
		);
	}

	// 3dgep does some bs func UpdateSubresources

	// --- CPU copy into the upload resource ---
	void* mappedPtr = nullptr;
	D3D12_RANGE readRange{ 0, 0 }; // we don't intend to read from this resource on the CPU
	execute_and_test_hresult(
		(*pIntermediateResource)->Map(0, &readRange, &mappedPtr) // to map means to get the address of the internal GPU address, kind of idk
	);
	memcpy(mappedPtr, bufferData, bufferSize); // copy my manual data read via cpu onto the buffer
	(*pIntermediateResource)->Unmap(0, nullptr); // release the mappedPtr view thingy

	// --- GPU copy: upload -> default heap ---
	command_list->CopyBufferRegion(
		*pDestinationResource, 0,
		*pIntermediateResource, 0,
		bufferSize);
}
