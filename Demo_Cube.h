#pragma once

#include "badWin32.h"

#include <DirectXMath.h>
#include <vector>


#include "IGame.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Stopwatch.h"
#include "CommandQueue.h"
#include "RenderWindow.h"

// todo:: include redistributable direct x bullshit in the soruce ( also goes for GPU_CORE shit )

class DemoCube :public IGame
{
public:
	
	DemoCube();
	~DemoCube()override;

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
	
	void set_transition_barrier(ViewPtr<ID3D12GraphicsCommandList2> command_list, ViewPtr<ID3D12Resource> back_buffer, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	void clearRTV(ViewPtr<ID3D12GraphicsCommandList2> command_list, D3D12_CPU_DESCRIPTOR_HANDLE desc, FLOAT* clear_color);

	void clearDSV(ViewPtr<ID3D12GraphicsCommandList2> command_list, D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth);


	// Create a GPU buffer.
	void update_buffer_resource(ViewPtr<ID3D12GraphicsCommandList2> command_list,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
		size_t numElements, size_t elementSize, const void* bufferData,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	// Resize the depth buffer to match the size of the client area.
	void resize_depth_buffer(int width, int height);
private:
	FLOAT color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	Keyboard kb;
	Mouse mouse;

	ViewPtr<ID3D12Device4>    mDevice;
	ViewPtr<CommandQueue> mDireectCommandQueue;
	ViewPtr<RenderWindow> mWindow;

	std::vector<UINT64> mSignalTracker;
	Stopwatch mTimer;


	// vertex buffer for the cube
	Microsoft::WRL::ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	// index buffer for the cube
	Microsoft::WRL::ComPtr<ID3D12Resource> mIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
	
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

	DirectX::XMMATRIX mModelMatrix;
	DirectX::XMMATRIX mViewMatrix;
	DirectX::XMMATRIX mProjectionMatrix;

	bool mContentLoaded;
};