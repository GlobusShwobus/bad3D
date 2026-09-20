#pragma once
//
//#include "App/badWin32.h"
//
//#include <d3d12.h>
//#include <dxgi1_6.h>
//#include <DirectXMath.h>
//
//#include <vector>
//
//
//#include "App/IGame.h"
//
//#include "Tools/Stopwatch.h"
//#include "D3D12/CommandQueue.h"
//#include "D3D12/SwapChain.h"
//
//#include "Model/Mesh.h"
//#include "D3D12/Resource.h"
//
//class DemoCube2 :public IGame
//{
//	struct VertexPosColor
//	{
//		DirectX::XMFLOAT3 Position;
//		DirectX::XMFLOAT3 Color;
//	};
//
//public:
//
//	DemoCube2();
//	~DemoCube2()override;
//
//	void load_content() override;
//	void unload_content() override;
//
//	void on_update() override;
//	void on_render() override;
//	void on_resize(int w, int h) override;
//
//protected:
//
//	void kb_resolve();
//
//	void mouse_resolve();
//
//	// Resize the depth buffer to match the size of the client area.
//	void resize_depth_buffer(int width, int height);
//
//	std::vector<VertexPosColor> pyramid_vertex();
//	std::vector<WORD> pyramid_index();
//
//	std::vector<VertexPosColor> cube_vertex();
//	std::vector<WORD> cube_index();
//
//	std::vector<VertexPosColor> tetrahedron_vertex();
//	std::vector<WORD> tetrahedron_index();
//
//	std::vector<VertexPosColor> octahedron_vertex();
//	std::vector<WORD> octahedron_index();
//
//	std::vector<VertexPosColor> prism_vertex();
//	std::vector<WORD> prism_index();
//
//	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> prepare_buffers(ID3D12Device4* device, ID3D12GraphicsCommandList2* list);
//
//private:
//	FLOAT color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
//
//	ViewPtr<ID3D12Device4> mDevice;
//	ViewPtr<CommandQueue>  mDireectCommandQueue;
//	ViewPtr<SwapChain>     mSwapChain;
//
//	// CPU side cube data
//	VertexBuffer mVertexBuffer;
//	IndexBuffer mIndexBuffer;
//	Mesh mMeshViews[5];
//
//	// depth buffer
//	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthBuffer;
//	// desc heap for the depth buffer
//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDSVHeap;
//
//	// root sig
//	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
//
//	// pipeline state object
//	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
//
//	D3D12_VIEWPORT mViewport;
//	D3D12_RECT mScissorRect;
//
//	float mFOV;
//	int camX;
//	int camY;
//	int camZ;
//
//	DirectX::XMMATRIX mModelMatrix[5];
//	DirectX::XMMATRIX mViewMatrix;
//	DirectX::XMMATRIX mProjectionMatrix;
//
//	bool mRunning;
//};