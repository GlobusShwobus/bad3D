#include "Demo_Cube.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <algorithm>

#include "Utils.h"
#include "Application.h"

#include <d3dx12.h>

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

void DemoCube::load_content()
{
	auto& app = Application::instance();

	mDevice = app.get_device();
	mDireectCommandQueue = app.get_command_queue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	mWindow = app.get_window();

	auto copy_command_queue = app.get_command_queue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto copy_command_list = copy_command_queue->acquire_command_list();

	// upload vertex buffer data and assign the view data
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateVertexBuffer; // a temporary, make sure to stall the CPU util copy command queue is donzo
	update_buffer_resource(
		copy_command_list.command_list.Get(),
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
	Microsoft::WRL::ComPtr<ID3D12Resource> internmeduateIndexBuffer;
	update_buffer_resource(
		copy_command_list.command_list.Get(),
		&mIndexBuffer,
		&internmeduateIndexBuffer,
		_countof(gCubeIndicies),
		sizeof(WORD),
		gCubeIndicies
	);

	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	mIndexBufferView.SizeInBytes = sizeof(gCubeIndicies);

	// create the descriptor heap for the depth stencil view
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	
	execute_and_test_hresult(
		mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDSVHeap))
	);

	// load the vertex shader and pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
	execute_and_test_hresult(
		D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob)
	);
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
	execute_and_test_hresult(
		D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderBlob)
	);

	// Create the vertex input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// create a root signature
	// check for root sig version, 1.1 is recommended
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(mDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}
	
	// allow input layout and deny unnecessary acces to certain pipeline stages
	D3D12_ROOT_SIGNATURE_FLAGS rootsigflags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;


	// root sig desc
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootsigdesc = {};
	rootsigdesc.Version = featureData.HighestVersion;

	if (featureData.HighestVersion == D3D_ROOT_SIGNATURE_VERSION_1_1)
	{
		D3D12_ROOT_PARAMETER1 rootParameters11[1];
		rootParameters11[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameters11[0].Constants.ShaderRegister = 0;                        // b0
		rootParameters11[0].Constants.RegisterSpace = 0;                         // space0
		rootParameters11[0].Constants.Num32BitValues = sizeof(DirectX::XMMATRIX) / 4;     // 16
		rootParameters11[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		rootsigdesc.Desc_1_1.NumParameters = _countof(rootParameters11);
		rootsigdesc.Desc_1_1.pParameters = rootParameters11;
		rootsigdesc.Desc_1_1.NumStaticSamplers = 0;
		rootsigdesc.Desc_1_1.pStaticSamplers = nullptr;
		rootsigdesc.Desc_1_1.Flags = rootsigflags;
	}
	else
	{
		D3D12_ROOT_PARAMETER rootParameters10[1];
		rootParameters10[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameters10[0].Constants.ShaderRegister = 0;
		rootParameters10[0].Constants.RegisterSpace = 0;
		rootParameters10[0].Constants.Num32BitValues = sizeof(DirectX::XMMATRIX) / 4;
		rootParameters10[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		rootsigdesc.Desc_1_0.NumParameters = _countof(rootParameters10);
		rootsigdesc.Desc_1_0.pParameters = rootParameters10;
		rootsigdesc.Desc_1_0.NumStaticSamplers = 0;
		rootsigdesc.Desc_1_0.pStaticSamplers = nullptr;
		rootsigdesc.Desc_1_0.Flags = rootsigflags;
	}

	// serialize
	Microsoft::WRL::ComPtr<ID3DBlob> rootSigBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	execute_and_test_hresult(
		D3D12SerializeVersionedRootSignature(&rootsigdesc, &rootSigBlob, &errorBlob)
	);

	execute_and_test_hresult(
		mDevice->CreateRootSignature( 0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature))
	);

	// pipeline state object
	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
	} pipelineStateStream;

	pipelineStateStream.pRootSignature = mRootSignature.Get();
	pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};

	execute_and_test_hresult(
		mDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&mPipelineState))
	);
	
	auto fenceVal = copy_command_queue->execute( copy_command_list );
	copy_command_queue->wait(fenceVal);

	//other
	// 
	// scissor rect is responsible for culling any pixels that are not within the dimensions of the RT
	RECT client_rect = mWindow->get_client_rect();
	const UINT client_width = static_cast<UINT>(rect_width(client_rect));
	const UINT client_height = static_cast<UINT>(rect_height(client_rect));
	mScissorRect = D3D12_RECT{ 0,0,LONG_MAX, LONG_MAX };

	// viewport rect is responsible for saying where to write to but it should not be outside the RT
	mViewport = D3D12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(client_width), static_cast<float>(client_height), 0.0f, 1.0f };

	// represents the vertical vield of view of the camera (it looks like a cone but not really, it kind of scales shit instead)
	mFOV = 45.0f;

	mSignalTracker.resize(mWindow->get_buffer_count(), 0);
	mTimer.reset();

	mContentLoaded = true;

	// resize/ create the depth buffer
	resize_depth_buffer(client_width, client_height);
}

void DemoCube::unload_content()
{
	if (mContentLoaded) {
		auto& app = Application::instance();
		app.flush();

		mDevice = nullptr;
		mDireectCommandQueue = nullptr;
		mWindow = nullptr;

		mVertexBuffer.Reset();
		mIndexBuffer.Reset();
		mDepthBuffer.Reset();
		mDSVHeap.Reset();
		mRootSignature.Reset();
		mPipelineState.Reset();
	
		mContentLoaded = false;
	}
}

void DemoCube::on_update()
{
	const float dt = mTimer.dt_float();

	mouse_resolve();
	kb_resolve();
	mouse.update_mouse_buttons(dt);

	// this demo specific:
	static double totalTime = 0;
	totalTime += dt;

	// update the model matrix
	float angle = static_cast<float>(totalTime * 90.0);
	const DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);
	mModelMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));

	// Update the view matrix.
	const DirectX::XMVECTOR eyePosition = DirectX::XMVectorSet(0, 0, -10, 1);
	const DirectX::XMVECTOR focusPoint  = DirectX::XMVectorSet(0, 0, 0, 1);
	const DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
	mViewMatrix = DirectX::XMMatrixLookAtLH(eyePosition,focusPoint,upDirection);

	// update the proj matrix
	RECT client_rect = mWindow->get_client_rect();
	UINT client_width = static_cast<UINT>(rect_width(client_rect));
	UINT client_height = static_cast<UINT>(rect_height(client_rect));

	client_height = std::max(1u, client_height);
	float aspectRatio = client_width / static_cast<float>(client_height);

	mProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(mFOV), aspectRatio, 0.1f, 100.0f);
}

void DemoCube::on_render() 
{
	auto command_context = mDireectCommandQueue->acquire_command_list();
	ID3D12GraphicsCommandList2* command_list = command_context.command_list.Get();
	ViewPtr<ID3D12Resource> current_back_buffer = mWindow->get_buffer();
	D3D12_CPU_DESCRIPTOR_HANDLE buffer_desc = mWindow->get_buffer_desc();
	D3D12_CPU_DESCRIPTOR_HANDLE dsv_desc = mDSVHeap->GetCPUDescriptorHandleForHeapStart();

	set_transition_barrier(command_list, current_back_buffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// any rendering logic goes here until another transition barrier
	clearRTV(command_list, buffer_desc, color);
	clearDSV(command_list, dsv_desc, 1.0f);

	// pre stuff, in this case vertex and pixel shaders stuff
	command_list->SetPipelineState(mPipelineState.Get());
	command_list->SetGraphicsRootSignature(mRootSignature.Get());
	// input assembler
	command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	command_list->IASetVertexBuffers(0,1,&mVertexBufferView);
	command_list->IASetIndexBuffer(&mIndexBufferView);
	// rasteriser state
	command_list->RSSetViewports(1, &mViewport);
	command_list->RSSetScissorRects(1, &mScissorRect);
	// output merger state
	command_list->OMSetRenderTargets(1,&buffer_desc, FALSE, &dsv_desc);

	// update the MVP matrix
	DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixMultiply(mModelMatrix, mViewMatrix);
	mvpMatrix = DirectX::XMMatrixMultiply(mvpMatrix, mProjectionMatrix);
	command_list->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / 4, &mvpMatrix, 0);

	// draw
	command_list->DrawIndexedInstanced(_countof(gCubeIndicies),1,0,0,0);

	// present
	set_transition_barrier(command_list, current_back_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	const UINT64 current_index = mWindow->get_buffer_index();
	const UINT64 signal_val = mDireectCommandQueue->execute( command_context );
	mSignalTracker[current_index] = signal_val;

	mWindow->present_to_display();

	const UINT64 some_new_buffer_index = mWindow->get_buffer_index();

	mDireectCommandQueue->wait(mSignalTracker[some_new_buffer_index]);
}

void DemoCube::on_resize() 
{
	RECT client_rect = mWindow->get_client_rect();

	const UINT buffer_width = mWindow->get_buffer_width();
	const UINT buffer_height = mWindow->get_buffer_height();
	const UINT client_width = static_cast<UINT>(rect_width(client_rect));
	const UINT client_height = static_cast<UINT>(rect_height(client_rect));

	if (buffer_width != client_width || buffer_height != client_height)
	{
		mDireectCommandQueue->flush();

		const UINT current_val = mSignalTracker[mWindow->get_buffer_index()];

		for (auto& fence_val : mSignalTracker)
			fence_val = current_val;

		mWindow->resize(client_width, client_height);

		// this demo specific:
		mViewport = CD3DX12_VIEWPORT(0.0f,0.0f, static_cast<float>(client_width), static_cast<float>(client_height));
		resize_depth_buffer(client_width,client_height);
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

void DemoCube::set_transition_barrier(ViewPtr<ID3D12GraphicsCommandList2> command_list, ViewPtr<ID3D12Resource> back_buffer, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
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

void DemoCube::clearRTV(ViewPtr<ID3D12GraphicsCommandList2> command_list, D3D12_CPU_DESCRIPTOR_HANDLE desc, FLOAT* clear_color)
{
	command_list->ClearRenderTargetView(desc, clear_color, 0, nullptr);
}

void DemoCube::clearDSV(ViewPtr<ID3D12GraphicsCommandList2> command_list, D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
	command_list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void DemoCube::update_buffer_resource(ViewPtr<ID3D12GraphicsCommandList2> command_list,
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

void DemoCube::resize_depth_buffer(int width, int height)
{
	if (mContentLoaded)
	{
		Application::instance().flush();

		width = std::max(1, width);
		height = std::max(1, height);

		// resize screen dependent resources
		// create depth buffer
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		optimizedClearValue.DepthStencil = { 1.0f,0 };

		const D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		const D3D12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_D32_FLOAT,
			width, height,
			1, 0, 1, 0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);

		execute_and_test_hresult(
			mDevice->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&depthDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&optimizedClearValue,
				IID_PPV_ARGS(&mDepthBuffer)
			)
		);

		// update the depth stencil view
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = DXGI_FORMAT_D32_FLOAT;
		dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		dsv.Flags = D3D12_DSV_FLAG_NONE;

		mDevice->CreateDepthStencilView(mDepthBuffer.Get(), &dsv, mDSVHeap->GetCPUDescriptorHandleForHeapStart());	
	}
}
