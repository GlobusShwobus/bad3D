#pragma once

#include "badWin32.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

#include <vector>


#include "IGame.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Stopwatch.h"
#include "CommandQueue.h"
#include "RenderWindow.h"

#include "Mesh.h"


class DemoCube2 :public IGame
{
	struct VertexPosColor
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Color;
	};

public:

	DemoCube2();
	~DemoCube2()override;

	void load_content() override;
	void unload_content() override;

	void on_update() override;
	void on_render() override;
	void on_resize() override;

	void on_key_event(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void on_mouse_event(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

protected:

	void kb_resolve();

	void mouse_resolve();

	// Resize the depth buffer to match the size of the client area.
	void resize_depth_buffer(int width, int height);

	std::vector<VertexPosColor> cpu_vertex_buffer();
	std::vector<WORD> cpu_index_buffer();
private:
	FLOAT color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	Keyboard kb;
	Mouse mouse;

	ViewPtr<ID3D12Device4>    mDevice;
	ViewPtr<CommandQueue> mDireectCommandQueue;
	ViewPtr<RenderWindow> mWindow;

	std::vector<UINT64> mSignalTracker;
	Stopwatch mTimer;

	// CPU side cube data
	Mesh mCubeMesh;

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

	bool mContentLoaded;
};