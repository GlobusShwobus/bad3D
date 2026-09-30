#pragma once

#include "App/badWin32.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

#include <vector>


#include "Demo/IGame.h"

#include "Tools/Stopwatch.h"
#include "App/CommandQueue.h"
#include "App/RenderWindow.h"

#include "Model/Mesh.h"
#include "D3D12/Resource.h"

class DemoCube2 final :public IScene
{
	struct VertexPosColor
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Color;
	};

public:

	DemoCube2(const AppState& read_events)
		:IScene(read_events)
	{
		// check of directX math library support
		if (!DirectX::XMVerifyCPUSupport())
		{
			throw std::runtime_error("memes");
		}
	}
	~DemoCube2() override
	{
		// careful because on_update() in this demo unloads itself which is the better behavior than on destructor
		// unload_content();
	}

	void load_content(GraphicsDevice* device, RenderWindow* window) override;
	void unload_content() override;

	void on_update() override;
	void on_render() override;
	void on_resize(int w, int h) override;

protected:

	void kb_resolve();

	void mouse_resolve();

	// Resize the depth buffer to match the size of the client area.
	void resize_depth_buffer(int width, int height);

	std::vector<VertexPosColor> pyramid_vertex();
	std::vector<WORD> pyramid_index();

	std::vector<VertexPosColor> cube_vertex();
	std::vector<WORD> cube_index();

	std::vector<VertexPosColor> tetrahedron_vertex();
	std::vector<WORD> tetrahedron_index();

	std::vector<VertexPosColor> octahedron_vertex();
	std::vector<WORD> octahedron_index();

	std::vector<VertexPosColor> prism_vertex();
	std::vector<WORD> prism_index();

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> prepare_buffers(ID3D12Device4* device, ID3D12GraphicsCommandList2* list);

private:
	FLOAT color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

	ViewPtr<GraphicsDevice> mGraphicsDevice;
	ViewPtr<ID3D12Device4> mDevice;
	ViewPtr<RenderWindow>  mRenderWindow;

	// awooga 
	std::unique_ptr<Mesh> mMesh;

	// depth buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthBuffer;
	// desc heap for the depth buffer
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDSVHeap;

	// root sig
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;

	// pipeline state object
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	float mFOV;
	int camX;
	int camY;
	int camZ;

	DirectX::XMMATRIX mModelMatrix[5];
	DirectX::XMMATRIX mViewMatrix;
	DirectX::XMMATRIX mProjectionMatrix;

	bool mRunning;
};