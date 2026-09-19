#include "DemoCube2.h"

#include <algorithm>

#include "App/Application.h"
#include <d3dcompiler.h>
#include "D3D12/EasyDirectX.h"
#include "D3D12/EasyDirectXUtils.h"
#include <array>

DemoCube2::DemoCube2()
{
	// check of directX math library support
	if (!DirectX::XMVerifyCPUSupport())
	{
		throw std::runtime_error("memes");
	}
}
DemoCube2::~DemoCube2()
{
	// careful because on_update() in this demo unloads itself which is the better behavior than on destructor
	// unload_content();
}

void DemoCube2::load_content()
{
	// gets
	auto& app = Application::instance();

	mDevice = ViewPtr{ app.get_device() };
	mDireectCommandQueue = ViewPtr{ app.get_command_queue(D3D12_COMMAND_LIST_TYPE_DIRECT) };
	mWindow = ViewPtr{ app.get_render_window() };

	auto copy_command_queue = app.get_command_queue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto copy_command_list = copy_command_queue->acquire_command_list();

	// prepare mesh
	auto uploads = prepare_buffers(mDevice.get(), copy_command_list.command_list.Get());

	// create the descriptor heap for the depth stencil view
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = DESCRIPTOR_HEAP_DESC::DSV(1);

	execute_and_test_hresult(
		mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDSVHeap))
	);

	// load the vertex shader and pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
	execute_and_test_hresult(
		D3DReadFileToBlob(L"DemoCube2VS.cso", &vertexShaderBlob)
	);
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
	execute_and_test_hresult(
		D3DReadFileToBlob(L"DemoCube2PS.cso", &pixelShaderBlob)
	);
	// the root signature is embedded in the vertex shader itself. extract that part from the blob and create the root sig
	Microsoft::WRL::ComPtr<ID3DBlob> root_sig_blob;
	execute_and_test_hresult(
		D3DReadFileToBlob(L"DemoCubeRootSig.bin", &root_sig_blob)
	);

	//execute_and_test_hresult(
	//	D3DGetBlobPart(
	//		vertexShaderBlob->GetBufferPointer(),
	//		vertexShaderBlob->GetBufferSize(),
	//		D3D_BLOB_ROOT_SIGNATURE,
	//		0,
	//		&root_sig_blob
	//	)
	//);

	execute_and_test_hresult(
		mDevice->CreateRootSignature(0, root_sig_blob->GetBufferPointer(), root_sig_blob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature))
	);

	// Create the vertex input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		// intentionally making a different order, should never actually write code like this but it is possible
		INPUT_ELEMENT::color(0, DXGI_FORMAT_R32G32B32_FLOAT,0, sizeof(DirectX::XMFLOAT3)),
		INPUT_ELEMENT::position(0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0)
	};


	// pipeline state object
	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	struct PipelineStateStream
	{
		PSS::ROOT_SIGNATURE pRootSignature;
		PSS::INPUT_LAYOUT InputLayout;
		PSS::PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		PSS::RASTERIZER Rasterizer;
		PSS::VERTEX_SHADER VS;
		PSS::PIXEL_SHADER PS;
		PSS::DSV_FORMAT DSVFormat;
		PSS::RTV_FORMATS RTVFormats;
	} pipelineStateStream;

	pipelineStateStream.pRootSignature = mRootSignature.Get();
	pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.Rasterizer = RASTERIZER_DESC::solid_backcull();
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

	auto fenceVal = copy_command_queue->execute(copy_command_list);
	copy_command_queue->wait_CPU(fenceVal);

	//other
	// 
	// scissor rect is responsible for culling any pixels that are not within the dimensions of the RT

	mScissorRect = D3D12_RECT{ 0,0,LONG_MAX, LONG_MAX };

	// viewport rect is responsible for saying where to write to but it should not be outside the RT
	mViewport = D3D12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(mWindow->get_buffer_width()), static_cast<float>(mWindow->get_buffer_height()), 0.0f, 1.0f };

	// represents the vertical vield of view of the camera (it looks like a cone but not really, it kind of scales shit instead)
	mFOV = 45.0f;
	camX = 0;
	camY = 0;
	camZ = -10;

	// resize/ create the depth buffer
	resize_depth_buffer(mWindow->get_buffer_width(), mWindow->get_buffer_height());

	mRunning = true;
}

void DemoCube2::unload_content()
{
	auto& app = Application::instance();
	app.flush();

	mDevice = nullptr;
	mDireectCommandQueue = nullptr;
	mWindow = nullptr;

	mDepthBuffer.Reset();
	mDSVHeap.Reset();
	mRootSignature.Reset();
	mPipelineState.Reset();

	// optionally can call shutdown here too
	// app.shutdown();
}

void DemoCube2::on_update()
{
	auto& app = Application::instance();
	const auto& events = app.get_state_manager();

	if (events.System().SystemQuitEvent())
	{
		mRunning = false;
		app.exit_loop();
	}
	
	if (mRunning)
	{
		mouse_resolve();
		kb_resolve();

		// check if resize
		if (events.System().WindowResizeEvent())
		{
			on_resize(events.System().WindowResizeWidth(), events.System().WindowResizeHeight());
		}


		// update the model matrix
		float angle = static_cast<float>(events.System().Age() * 90.0);
		const DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);
		DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));

		// simple spread along X so the 5 cubes don't overlap
		constexpr float spacing = 3.0f;
		for (int i = 0; i < 5; ++i)
		{
			float offsetX = (i - 2) * spacing; // centers the row: -2,-1,0,1,2 * spacing
			DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(offsetX, 0.0f, 0.0f);
			mModelMatrix[i] = DirectX::XMMatrixMultiply(rotation, translation);
		}

		// Update the view matrix.
		const DirectX::XMVECTOR eyePosition = DirectX::XMVectorSet(camX, camY, camZ, 1);
		const DirectX::XMVECTOR focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
		const DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
		mViewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

		// update the proj matrix
		UINT client_width = mWindow->get_buffer_width();
		UINT client_height = mWindow->get_buffer_height();

		client_height = std::max(1u, client_height);
		float aspectRatio = client_width / static_cast<float>(client_height);

		mProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(mFOV), aspectRatio, 0.1f, 100.0f);
	}
	else
	{
		unload_content();
	}
}

void DemoCube2::on_render()
{
	if (mRunning)
	{
		auto command_context = mDireectCommandQueue->acquire_command_list();
		ID3D12GraphicsCommandList2* command_list = command_context.command_list.Get();
		ID3D12Resource* current_back_buffer = mWindow->get_buffer();
		D3D12_CPU_DESCRIPTOR_HANDLE buffer_desc = mWindow->get_buffer_desc();
		D3D12_CPU_DESCRIPTOR_HANDLE dsv_desc = mDSVHeap->GetCPUDescriptorHandleForHeapStart();

		command_context.transition(current_back_buffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// any rendering logic goes here until another transition barrier

		command_context.clear_RTV(buffer_desc, color);
		command_context.clear_DSV(dsv_desc, 1.0f);

		// THIS STUFF IS SET ONCE
		// pre stuff, in this case vertex and pixel shaders stuff
		command_list->SetPipelineState(mPipelineState.Get());
		command_list->SetGraphicsRootSignature(mRootSignature.Get());
		// input assembler
		command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// rasteriser state
		command_list->RSSetViewports(1, &mViewport);
		command_list->RSSetScissorRects(1, &mScissorRect);
		// output merger state
		command_list->OMSetRenderTargets(1, &buffer_desc, FALSE, &dsv_desc);
		// root constants
		const UINT matrix_32bit_value_count = sizeof(DirectX::XMMATRIX) / sizeof(UINT); //16
		command_list->SetGraphicsRoot32BitConstants(0, matrix_32bit_value_count, &mViewMatrix, 0);
		command_list->SetGraphicsRoot32BitConstants(1, matrix_32bit_value_count, &mProjectionMatrix, 0);

		// THIS STUFF IS PER OBJECT
		for (int i = 0; i < 5; ++i)
		{
			auto& mesh = mMeshViews[i];

			command_list->IASetVertexBuffers(0, 1, &mesh.get_vertex_view());
			command_list->IASetIndexBuffer(&mesh.get_index_view());
			command_list->SetGraphicsRoot32BitConstants(2, matrix_32bit_value_count, &mModelMatrix[i], 0);
			command_list->DrawIndexedInstanced(mesh.get_index_count(), 1, 0, 0, 0);
		}

		// present
		command_context.transition(current_back_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		const UINT64 current_index = mWindow->get_buffer_index();
		const UINT64 signal_val = mDireectCommandQueue->execute(command_context);

		UINT64 next_buffer_signal = mWindow->present_to_display(signal_val);

		mDireectCommandQueue->wait_CPU(next_buffer_signal);
	}
}

void DemoCube2::on_resize(int w, int h)
{
	w = std::max(1, w);
	h = std::max(1, h);

	const UINT buffer_width = mWindow->get_buffer_width();
	const UINT buffer_height = mWindow->get_buffer_height();

	if (buffer_width != w || buffer_height != h)
	{
		mWindow->resize(*mDireectCommandQueue, w, h);

		// this demo specific:
		mViewport = D3D12_VIEWPORT{ 0.0f,0.0f, static_cast<float>(w), static_cast<float>(h), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
		resize_depth_buffer(w, h);
	}
}

void DemoCube2::kb_resolve()
{
	auto& app = Application::instance();
	const auto& events = app.get_state_manager();

	static bool fullscreen = false;
	static bool f11_previous = false;


	const bool* keys = events.Keyboard().GetKeys();

	const bool f11_current = keys[VK_F11];

	if (f11_current && !f11_previous)
	{
		fullscreen = !fullscreen;
		mWindow->toggle_fullscreen(fullscreen);
	}

	f11_previous = f11_current;

	if (keys['W'])
	{
		camY += 1;
	}
	if (keys['A'])
	{
		camX -= 1;
	}
	if (keys['S'])
	{
		camY -= 1;
	}
	if (keys['D'])
	{
		camX += 1;
	}

	if (keys['Q'])
	{
		camZ -= 1;
	}
	if (keys['E'])
	{
		camZ += 1;
	}
}

void DemoCube2::mouse_resolve()
{
	auto& app = Application::instance();
	const auto& events = app.get_state_manager();
	mFOV += events.Mouse().WheelDeltaNormalized();

	if (mFOV < 1) // !!!!!!! crashes if fov is 0
	{
		mFOV = 1;
	}
	else if(mFOV>180)
	{
		mFOV = 180;
	}
}


void DemoCube2::resize_depth_buffer(int width, int height)
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
		mDevice.get(),
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

std::vector<DemoCube2::VertexPosColor> DemoCube2::pyramid_vertex()
{
	return std::vector<VertexPosColor>{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0 base
		{ DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 1 base
		{ DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f),  DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f) }, // 2 base
		{ DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f),  DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3 base
		{ DirectX::XMFLOAT3(0.0f,  1.0f,  0.0f),  DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) }  // 4 apex	
	};
}

std::vector<WORD> DemoCube2::pyramid_index()
{
	return std::vector<WORD>{
		0, 3, 2, 0, 2, 1,
			0, 1, 4,
			1, 2, 4,
			2, 3, 4,
			3, 0, 4
	};
}

std::vector<DemoCube2::VertexPosColor> DemoCube2::cube_vertex()
{
	return std::vector<VertexPosColor>{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
		{ DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
		{ DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f),  DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
		{ DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f),  DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
		{ DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
		{ DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
		{ DirectX::XMFLOAT3(1.0f,  1.0f,  1.0f),  DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
		{ DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f),  DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7		
	};
}

std::vector<WORD> DemoCube2::cube_index()
{
	return std::vector<WORD>{
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7
	};
}

std::vector<DemoCube2::VertexPosColor> DemoCube2::tetrahedron_vertex()
{
	return std::vector<VertexPosColor>{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
		{ DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f),  DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 1
		{ DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f),  DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f) }, // 2
		{ DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f) }  // 3
	};
}

std::vector<WORD> DemoCube2::tetrahedron_index()
{
	return std::vector<WORD>{
		0, 1, 2,
			0, 2, 3,
			0, 3, 1,
			1, 3, 2
	};
}

std::vector<DemoCube2::VertexPosColor> DemoCube2::octahedron_vertex()
{
	return std::vector<VertexPosColor>{
		{ DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT3(0.5f, 1.0f, 0.5f) }, // 0 top
		{ DirectX::XMFLOAT3(1.0f,  0.0f,  0.0f), DirectX::XMFLOAT3(1.0f, 0.5f, 0.5f) }, // 1 +x
		{ DirectX::XMFLOAT3(0.0f,  0.0f,  1.0f), DirectX::XMFLOAT3(0.5f, 0.5f, 1.0f) }, // 2 +z
		{ DirectX::XMFLOAT3(-1.0f, 0.0f,  0.0f), DirectX::XMFLOAT3(0.0f, 0.5f, 0.5f) }, // 3 -x
		{ DirectX::XMFLOAT3(0.0f,  0.0f, -1.0f), DirectX::XMFLOAT3(0.5f, 0.5f, 0.0f) }, // 4 -z
		{ DirectX::XMFLOAT3(0.0f, -1.0f,  0.0f), DirectX::XMFLOAT3(0.5f, 0.0f, 0.5f) }  // 5 bottom
	};
}

std::vector<WORD> DemoCube2::octahedron_index()
{
	return std::vector<WORD>{
		0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 1,
			5, 2, 1,
			5, 3, 2,
			5, 4, 3,
			5, 1, 4
	};
}

std::vector<DemoCube2::VertexPosColor> DemoCube2::prism_vertex()
{
	return std::vector<VertexPosColor>{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0 front-left
		{ DirectX::XMFLOAT3(1.0f,  -1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 1 front-right
		{ DirectX::XMFLOAT3(0.0f,   1.0f, -1.0f), DirectX::XMFLOAT3(0.5f, 1.0f, 0.0f) }, // 2 front-top
		{ DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 3 back-left
		{ DirectX::XMFLOAT3(1.0f,  -1.0f,  1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f) }, // 4 back-right
		{ DirectX::XMFLOAT3(0.0f,   1.0f,  1.0f), DirectX::XMFLOAT3(0.5f, 1.0f, 1.0f) }  // 5 back-top
	};
}

std::vector<WORD> DemoCube2::prism_index()
{
	return std::vector<WORD>{
		0, 2, 1, 
			3, 4, 5, 
			0, 1, 4,
			0, 4, 3,
			1, 2, 5,
			1, 5, 4,
			2, 0, 3,
			2, 3, 5
	};
}

std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> DemoCube2::prepare_buffers(ID3D12Device4* device, ID3D12GraphicsCommandList2* list)
{
	// setup variables
	const UINT PER_VERTEX_SIZE = sizeof(VertexPosColor);
	const UINT PER_INDEX_SIZE = sizeof(WORD);
	const DXGI_FORMAT INDEX_BUFFER_FORMAT = DXGI_FORMAT_R16_UINT;

	struct ShapeData
	{
		std::vector<VertexPosColor> vertices;
		std::vector<WORD> indices;
	};

	std::array<ShapeData, 5> shapes = {
		ShapeData{ pyramid_vertex(),     pyramid_index() },
		ShapeData{ cube_vertex(),        cube_index() },
		ShapeData{ tetrahedron_vertex(), tetrahedron_index() },
		ShapeData{ octahedron_vertex(),  octahedron_index() },
		ShapeData{ prism_vertex(),       prism_index() },
	};

	// calculate the total size in bytes for all shapes per vertex and per index buffers
	UINT vertex_bytes = 0;
	UINT index_bytes = 0;
	for (const auto& shape : shapes)
	{
		vertex_bytes += (static_cast<UINT>(shape.vertices.size()) * PER_VERTEX_SIZE);
		index_bytes += (static_cast<UINT>(shape.indices.size()) * PER_INDEX_SIZE);
	}

	// create vertex buffer
	mVertexBuffer = VertexBuffer{device, vertex_bytes, PER_VERTEX_SIZE };

	// create index buffer
	mIndexBuffer = IndexBuffer{ device, index_bytes, DXGI_FORMAT_R16_UINT };

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediaries;
	
	// after creating resources with the exact size required to store all shapes 2 more steps are required
	// 1) make a sub view of a specific shape. a range in other words
	// 2) upload the data to the GPU side
	// to achieve both tracking of bytes is required
	UINT64 vertex_buffer_offset = 0;
	UINT64 index_buffer_offset = 0;
	for (int i=0;i<shapes.size();i++)
	{
		auto& vertex_buffer = shapes[i].vertices;
		auto& index_buffer = shapes[i].indices;
		auto& mesh = mMeshViews[i];

		const UINT vertex_bytes = static_cast<UINT>(vertex_buffer.size()) * PER_VERTEX_SIZE; // cast because d3d12 itself isnt consistent
		const UINT index_bytes = static_cast<UINT>(index_buffer.size()) * PER_INDEX_SIZE;

		mesh = Mesh{
			ViewPtr<VertexBuffer>(&mVertexBuffer),
			vertex_buffer_offset,
			vertex_bytes,
			ViewPtr<IndexBuffer>(&mIndexBuffer), 
			index_buffer_offset, 
			index_bytes 
		};

		intermediaries.emplace_back(
			copy_buffer_to_resource_and_get_intermediary(
				device,
				list,
				mVertexBuffer.get(),
				vertex_buffer_offset,
				vertex_buffer.data(),
				vertex_bytes
			)
		);

		intermediaries.emplace_back(
			copy_buffer_to_resource_and_get_intermediary(
				device,
				list,
				mIndexBuffer.get(),
				index_buffer_offset,
				index_buffer.data(),
				index_bytes
			)
		);

		vertex_buffer_offset += vertex_bytes;
		index_buffer_offset += index_bytes;
	}

	return intermediaries;
}
