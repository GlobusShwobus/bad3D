#pragma once

#include "IGame.h"
#include <DirectXMath.h>
#include "ObserverPtr.h"

#include <stdexcept>
//	class Tutorial2 :public IGame
//	{
//	public:
//	
//		Tutorial2()
//		{
//			// check of directX math library support
//			if (!DirectX::XMVerifyCPUSupport())
//			{
//				throw std::runtime_error("memes");
//			}
//		}
//		~Tutorial2() = default;
//	
//		// content loading / unloading
//		void load_content() override;
//		void unload_content() override;
//	
//		// on game specific logic update and rendering update
//		void on_update(float dt) = 0;
//		void on_render(const RenderFrameContext& frame_context)
//		{
//			frame_context.command_list->ClearRenderTargetView(frame_context.resource_desc, clear_color, 0, nullptr);
//			frame_context.command_list->ClearDepthStencilView(mDSVHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f,0,0,nullptr);
//			frame_context.command_list->OMSetRenderTargets(1, &frame_context.resource_desc, FALSE, &mDSVHandle);
//		}
//	
//		// key and mouse events
//		void on_key_event(UINT uMsg, WPARAM wParam, LPARAM lParam) {}
//		void on_mouse_event(UINT uMsg, WPARAM wParam, LPARAM lParam) {}
//	
//		// Resize the depth buffer to match the size of the client area.
//		void on_window_resize(int width, int height) override
//		{
//			.
//		}
//	
//	private:
//		// Helper functions
//	// Transition a resource
//		//	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
//		//		Microsoft::WRL::ComPtr<ID3D12Resource> resource,
//		//		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
//		//	
//		//	// Clear a render target view.
//		//	void ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
//		//		D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);
//	
//		// Clear the depth of a depth-stencil view.
//		//void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
//		//	D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);
//	
//		// Create a GPU buffer.
//		void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
//			ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
//			size_t numElements, size_t elementSize, const void* bufferData,
//			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
//	
//		FLOAT clear_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
//		
//		D3D12Resource mDepthBuffer;
//		D3D12DescriptorHeap mDSVHeap;
//		D3D12_CPU_DESCRIPTOR_HANDLE mDSVHandle{};
//	};