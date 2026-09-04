#include "Demo_Cube.h"

#include <algorithm>

#include "Utils.h"
#include "Application.h"
#include "EasyDirectX.h"
#include "EasyDirectXUtils.h"

//TODO challange: this demo draws just one cube. strictly speaking it is wasteful to assign MVP matrix to GPU register b0 because the model changes per cube.
//                instead of binding MVP to register b0, try binding view and projection matricies separately ( or without model transformation ) then apply model transformation
//                in HLSL, for example by editing the shader to take main(vertex IN, matrix IN_model)

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
	// gets
	auto& app = Application::instance();

	mDevice = app.get_device();
	mDireectCommandQueue = app.get_command_queue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	mWindow = app.get_render_window();

	auto copy_command_queue = app.get_command_queue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto copy_command_list = copy_command_queue->acquire_command_list();

	// assign cube values to the mesh (since demo, is hardcoded. from file is cooler)
	set_mesh();

	// create vertex buffer then copy CPU side data to it then make view handle
	mVertexBuffer = create_commited_resource(
		mDevice,
		HEAP_PROPERTY::base(),
		RESOURCE_DESC::buffer(mCubeMesh.vertex_buffer_size()),
		D3D12_RESOURCE_STATE_COMMON
	);

	auto vertex_upload_resource = copy_buffer_to_resource_and_get_intermediary(
		mDevice,
		copy_command_list.command_list.Get(),
		mVertexBuffer.Get(),
		mCubeMesh.mVertexBuffer
	);

	mVertexBufferView = RESOURCE_VIEW::vertex(mVertexBuffer->GetGPUVirtualAddress(), mCubeMesh.vertex_buffer_size(), mCubeMesh.vertex_type_size());

	// create index buffer then copy CPU side data to it then make handle
	mIndexBuffer = create_commited_resource(
		mDevice,
		HEAP_PROPERTY::base(),
		RESOURCE_DESC::buffer(mCubeMesh.index_buffer_size()),
		D3D12_RESOURCE_STATE_COMMON
	);

	auto index_upload_resource = copy_buffer_to_resource_and_get_intermediary(
		mDevice,
		copy_command_list.command_list.Get(),
		mIndexBuffer.Get(),
		mCubeMesh.mIndexBuffer
	);

	mIndexBufferView = RESOURCE_VIEW::index(mIndexBuffer->GetGPUVirtualAddress(), mCubeMesh.index_buffer_size(), DXGI_FORMAT_R16_UINT);

	// create the descriptor heap for the depth stencil view
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = DESCRIPTOR_HEAP_DESC::DSV(1);
	
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
		INPUT_ELEMENT::position(0, DXGI_FORMAT_R32G32B32_FLOAT),
		INPUT_ELEMENT::color(0, DXGI_FORMAT_R32G32B32_FLOAT)
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
	D3D12_ROOT_SIGNATURE_FLAGS rootsigflags = ROOT_SIGNATURE_FLAGS::ALLOW_IA_MINIMAL;

	// root sig desc
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootsigdesc = {};
	rootsigdesc.Version = featureData.HighestVersion;

	if (featureData.HighestVersion == D3D_ROOT_SIGNATURE_VERSION_1_1)
	{
		D3D12_ROOT_PARAMETER1 rootParameters11[1] = {
			ROOT_PARAMETER::constant(D3D12_SHADER_VISIBILITY_VERTEX, 0,  sizeof(DirectX::XMMATRIX) / 4)
		};

		rootsigdesc.Desc_1_1 = ROOT_DESCRIPTION::description(_countof(rootParameters11), rootParameters11, rootsigflags);
	}
	else
	{
		D3D12_ROOT_PARAMETER rootParameters10[1];
		rootParameters10[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameters10[0].Constants = ROOT_PARAMETER::CONSTANT::constant(0, sizeof(DirectX::XMMATRIX) / 4);
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
		PSS::ROOT_SIGNATURE pRootSignature;
		PSS::INPUT_LAYOUT InputLayout;
		PSS::PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		PSS::VERTEX_SHADER VS;
		PSS::PIXEL_SHADER PS;
		PSS::DSV_FORMAT DSVFormat;
		PSS::RTV_FORMATS RTVFormats;
	} pipelineStateStream;

	pipelineStateStream.pRootSignature = mRootSignature.Get();
	pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	pipelineStateStream.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};

	execute_and_test_hresult(
		mDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&mPipelineState))
	);
	
	auto fenceVal = copy_command_queue->execute( copy_command_list );
	copy_command_queue->wait_CPU(fenceVal);

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

	command_context.transition(current_back_buffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// any rendering logic goes here until another transition barrier

	command_context.clear_RTV(buffer_desc, color);
	command_context.clear_DSV(dsv_desc, 1.0f);

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
	command_list->DrawIndexedInstanced(mCubeMesh.index_count(), 1, 0, 0, 0);

	// present
	command_context.transition(current_back_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	const UINT64 current_index = mWindow->get_buffer_index();
	const UINT64 signal_val = mDireectCommandQueue->execute( command_context );
	mSignalTracker[current_index] = signal_val;

	mWindow->present_to_display();

	const UINT64 some_new_buffer_index = mWindow->get_buffer_index();

	mDireectCommandQueue->wait_CPU(mSignalTracker[some_new_buffer_index]);
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
		mDireectCommandQueue->flush_execution();

		const UINT current_val = mSignalTracker[mWindow->get_buffer_index()];

		for (auto& fence_val : mSignalTracker)
			fence_val = current_val;

		mWindow->resize(client_width, client_height);

		// this demo specific:
		mViewport = D3D12_VIEWPORT{ 0.0f,0.0f, static_cast<float>(client_width), static_cast<float>(client_height), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
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

		D3D12_HEAP_PROPERTIES heap_property = HEAP_PROPERTY::base();
		D3D12_RESOURCE_DESC resource_desc = RESOURCE_DESC::texture2d(width, height, DXGI_FORMAT_D32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		mDepthBuffer = create_commited_resource(
			mDevice,
			heap_property,
			resource_desc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_HEAP_FLAG_NONE,
			&optimizedClearValue
		);

		// update the depth stencil view
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_view = RESOURCE_VIEW::textured2d_DSV(DXGI_FORMAT_D32_FLOAT);

		mDevice->CreateDepthStencilView(mDepthBuffer.Get(), &dsv_view, mDSVHeap->GetCPUDescriptorHandleForHeapStart());
	}
}

void DemoCube::set_mesh()
{
	mCubeMesh = Mesh<VertexPosColor>{
	{ // pos/ color
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
		{ DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
		{ DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f),  DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
		{ DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f),  DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
		{ DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
		{ DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
		{ DirectX::XMFLOAT3(1.0f,  1.0f,  1.0f),  DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
		{ DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f),  DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7		
	},
	{ // index
			0, 1, 2, 0, 2, 3,
			4, 6, 5, 4, 7, 6,
			4, 5, 1, 4, 1, 0,
			3, 2, 6, 3, 6, 7,
			1, 5, 6, 1, 6, 2,
			4, 0, 3, 4, 3, 7
	}
	};
}
