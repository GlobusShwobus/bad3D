#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

#include <DirectXMath.h>
#include <wrl.h>
//
//                                                 +---------------+
//                                                 |    FACTORY    |
//                                                 +---------------+
// 				   		  					           	   |
// 				   		   ________________________________|____________________________________
// 				   		  |																	    |
// 				   		  V																	    |
// 				   +---------------+														    |
// 				   |    ADAPTER    |														    |
// 				   +---------------+				   										    |
//  					  |																	    |
// 						  V																	    V
// 			      +================+     	     +-----------------------+             +--------------------+
// 			      ||    DEVICE    || ----------> |     COMMAND QUEUE     | ----------> |     SWAP CHAIN     |
// 			      +================+	         +-----------------------+             +--------------------+
// 
// 
// 
// 
// 
// 
//
//


// DXGI is mainly responsible for three things and the top most handle for doing so is the Factory. The factory enumerates ( identifies ) the adapters that exist in the system physically,
// creates said adapters and also creates swap chains.
// 
// Adapter - A view ( descriptor ) handle of the system. Used to create a device.
// Device - A write handle of the system. Used to create resources: descriptor heaps, command allocators, command lists, command queues, fences and some others...
// Swap chain - A buffer (surface) manager type. It manages what buffers get data written into and what gets passed to the display.
// 
// IMPORTANT!!! This system targets windows 10 minimally and dx12.
// 
// Supported Methods:
//
//	Factory:
//		CreateSoftwareAdapter   - Create an adapter interface that represents a software adapter.
//		CreateSwapChain         - should not be used... use factory2 swap chains...
//		EnumAdapters            - Enumerates the adapters ( video cards ).
//		GetWindowAssociation    - Get the window through which the user controls the transition to and from full screen. WARNING: in dx12 real fullscreen should not be used, instead do windowed borderless fullscreen because otherwise the homework is too annoying.
//		MakeWindowAssociation   - Allows DXGI to monitor an application's message queue for the alt-enter key sequence (which causes the application to switch from windowed to full screen or vice versa). WARNING: again, fullscreen should be handled manually not by DefWindowProc
//
//	Factory1:
//		IsCurrent - Informs an application of the possible need to re-create the factory and re-enumerate adapters.
//		EnumAdapters1 - Enumerates both adapters (video cards) with or without outputs.
//
//	Factory2:
//		CreateSwapChainForComposition  - Creates a swap chain that you can use to send Direct3D content into the DirectComposition API or a Xaml framework to compose in a window.
//		CreateSwapChainForCoreWindow   - Creates a swap chain that is associated with the CoreWindow object for the output window for the swap chain.
//		CreateSwapChainForHwnd         - Creates a swap chain that is associated with an HWND handle to the output window for the swap chain.
//		GetSharedResourceAdapterLuid   - Identifies the adapter on which a shared resource object was created.
//		RegisterOcclusionStatusEvent   - Registers to receive notification of changes in occlusion status by using event signaling.
//		RegisterOcclusionStatusWindow  - Registers an application window to receive notification messages of changes of occlusion status.
//		UnregisterOcclusionStatus      - Unregisters a window or an event to stop it from receiving notification when occlusion status changes.
//		IsWindowedStereoEnabled        - Determines whether to use stereo mode.
//		RegisterStereoStatusEvent      - Registers to receive notification of changes in stereo status by using event signaling.
//		RegisterStereoStatusWindow     - Registers an application window to receive notification messages of changes of stereo status.
//		UnregisterStereoStatus         - Unregisters a window or an event to stop it from receiving notification when stereo status changes.
//
//	Factory3:
//		GetCreationFlags - Gets the flags that were used when a Microsoft DirectX Graphics Infrastructure (DXGI) object was created.
//
//	Factory4:
//		EnumAdapterByLuid - Outputs the IDXGIAdapter for the specified LUID.
//		EnumWarpAdapter - Provides an adapter which can be provided to D3D12CreateDevice to use the WARP renderer.
// 
//	Factory5:
//		CheckFeatureSupport - Used to check for hardware feature support.
using DXFactory4 = Microsoft::WRL::ComPtr<IDXGIFactory4>;       
using DXFactory5 = Microsoft::WRL::ComPtr<IDXGIFactory5>;

// DXGI Adapter represents a display subsystem ( video card...either physical GPU, DAC's and video memory but also motherboard based subsystems ). 
// The adapter enumerates display subsystems which means discovering what subsystems exist on the machine.
// In simple terms, it's the identification ( a view ) of the system. To actually use the GPU, a device is required. To create a device an adapter is required.
// Create adapter1 then using COM interface query methodology to get a desired adapter and API from it. 
// 
// Adapter1 alloc - Adapter4 alloc - Adapter1 create -> Query interface adapter4 from adapter1 -> if success done, if not try adapter3...
// For win10 and dx12 aim for adapter4 or adapter3 minimally.
// 
// Supported Methods:
//
//	Adapter:
//		CheckInterfaceSupport - Checks whether the system supports a device interface for a graphics component. (doesn't work for dx11 / dx12)
//		EnumOutputs - Enumerate adapter (video card) outputs.
//		GetDesc - DXGI_ADAPTER_DESC {
//						WCHAR  Description[128];       - In windows10 dx12 just "Software Adapter". (in older systems something else)
//						UINT   VendorId;		       - In windows10 dx12 just 0. (in older systems something else)
//						UINT   DeviceId;		       - In windows10 dx12 just 0. (in older systems something else)
//						UINT   SubSysId;               - In windows10 dx12 just 0. (in older systems something else)
//						UINT   Revision;               - In windows10 dx12 just 0. (in older systems something else)
//						SIZE_T DedicatedVideoMemory;   - The number of bytes of dedicated video memory that are not shared with the CPU.
//						SIZE_T DedicatedSystemMemory;  - The number of bytes of dedicated system memory that are not shared with the CPU. This memory is allocated from available system memory at boot time.
//						SIZE_T SharedSystemMemory;     - The number of bytes of shared system memory. This is the maximum value of system memory that may be consumed by the adapter during operation.
//						LUID   AdapterLuid;            - A unique value that identifies the adapter. 
//					}
//
//	Adapter1:
//		GetDesc1 - Added: UINT Flags - A value of the DXGI_ADAPTER_FLAG enumerated type that describes the adapter type. The DXGI_ADAPTER_FLAG_REMOTE flag is reserved.
//
//	Adapter2:
//		GetDesc2 - Added: DXGI_GRAPHICS_PREEMPTION_GRANULARITY GraphicsPreemptionGranularity; - describes the granularity level at which the GPU can be preempted from performing its current graphics rendering task.
//				   Added: DXGI_COMPUTE_PREEMPTION_GRANULARITY  ComputePreemptionGranularity;  - describes the granularity level at which the GPU can be preempted from performing its current compute task.
//
//	Adapter3:
//		QueryVideoMemoryInfo - DXGI_QUERY_VIDEO_MEMORY_INFO {
//									UINT64 Budget;                  - Specifies the OS-provided video memory budget, in bytes, that the application should target.
//									UINT64 CurrentUsage;            - Specifies the application’s current video memory usage, in bytes.
//									UINT64 AvailableForReservation; - The amount of video memory, in bytes, that the application has available for reservation. 
//									UINT64 CurrentReservation;      - The amount of video memory, in bytes, that is reserved by the application
//								}
//		RegisterHardwareContentProtectionTeardownStatusEvent - Registers to receive notification of hardware content protection teardown events.
//		RegisterVideoMemoryBudgetChangeNotificationEvent     - This method establishes a correlation between a CPU synchronization object and the budget change event.
//		SetVideoMemoryReservation                            - This method sends the minimum required physical memory for an application, to the OS. 
//		UnregisterHardwareContentProtectionTeardownStatus    - Unregisters an event to stop it from receiving notification of hardware content protection teardown events.
//		UnregisterVideoMemoryBudgetChangeNotification        - This method stops notifying a CPU synchronization object whenever a budget change occurs. An application may switch back to polling the information regularly.
//
//	Adapter4:
//		GetDesc3 - Changed UINT Flags to DXGI_ADAPTER_FLAG3 Flags plus added some settings.
using DXAdapter1 = Microsoft::WRL::ComPtr<IDXGIAdapter1>;
using DXAdapter2 = Microsoft::WRL::ComPtr<IDXGIAdapter2>;
using DXAdapter3 = Microsoft::WRL::ComPtr<IDXGIAdapter3>;
using DXAdapter4 = Microsoft::WRL::ComPtr<IDXGIAdapter4>;

// The device represents a virtual adapter; it is used to create command allocators, command lists, command queues, fences, resources,
// pipeline state objects, heaps, root signatures, samplers, and many resource views.
// The adapter is a view, the device is the manipulator. Once a device is created the adapter handle is no longer required.
//
// Supported Methods:
//	Device:
//		CheckFeatureSupport                - Gets information about the features that are supported by the current graphics driver. 
//		CopyDescriptors                    - Copies descriptors from a source to a destination. 
//		CopyDescriptorsSimple              - Copies descriptors from a source to a destination.
//		CreateCommandAllocator             - Creates a command allocator object.
//		CreateCommandList                  - Creates a command list.
//		CreateCommandQueue                 - Creates a command queue.
//		CreateCommandSignature             - This method creates a command signature.
//		CreateCommittedResource            - Creates both a resource and an implicit heap, such that the heap is big enough to contain the entire resource, and the resource is mapped to the heap.
//		CreateComputePipelineState         - Creates a compute pipeline state object.
//		CreateConstantBufferView           - Creates a constant-buffer view for accessing resource data.
//		CreateDepthStencilView             - Creates a depth-stencil view for accessing resource data.
//		CreateDescriptorHeap               - Creates a descriptor heap object.
//		CreateFence                        - Creates a fence object.
//		CreateGraphicsPipelineState        - Creates a graphics pipeline state object.
//		CreateHeap                         - Creates a heap that can be used with placed resources and reserved resources.
//		CreatePlacedResource               - Creates a resource that is placed in a specific heap. Placed resources are the lightest weight resource objects available, and are the fastest to create and destroy.
//		CreateQueryHeap                    - Creates a query heap. A query heap contains an array of queries.
//		CreateRenderTargetView             - Creates a render-target view for accessing resource data. (ID3D12Device.CreateRenderTargetView)
//		CreateReservedResource             - Creates a resource that is reserved, and not yet mapped to any pages in a heap.
//		CreateRootSignature                - Creates a root signature layout.
//		CreateSampler                      - Create a sampler object that encapsulates sampling information for a texture.
//		CreateShaderResourceView           - Creates a shader-resource view for accessing data in a resource.
//		CreateSharedHandle                 - Creates a shared handle to a heap, resource, or fence object.
//		CreateUnorderedAccessView          - Creates a view for unordered accessing.
//		Evict                              - Enables the page-out of data, which precludes GPU access of that data.
//		GetAdapterLuid                     - Gets a locally unique identifier for the current device (adapter).
//		GetCopyableFootprints              - Gets a resource layout that can be copied. Helps the app fill-in D3D12_PLACED_SUBRESOURCE_FOOTPRINT and D3D12_SUBRESOURCE_FOOTPRINT when suballocating space in upload heaps.
//		GetCustomHeapProperties            - Divulges the equivalent custom heap properties that are used for non-custom heap types, based on the adapter's architectural properties.
//		GetDescriptorHandleIncrementSize   - Gets the size of the handle increment for the given type of descriptor heap. This value is typically used to increment a handle into a descriptor array by the correct amount.
//		GetDeviceRemovedReason             - Gets the reason that the device was removed.
//		GetNodeCount                       - Reports the number of physical adapters (nodes) that are associated with this device.
//		GetResourceAllocationInfo          - Gets the size and alignment of memory required for a collection of resources on this adapter.
//		GetResourceTiling                  - Gets info about how a tiled resource is broken into tiles.
//		MakeResident                       - Makes objects resident for the device.
//		OpenSharedHandle                   - Opens a handle for shared resources, shared heaps, and shared fences, by using HANDLE and REFIID.
//		OpenSharedHandleByName             - Opens a handle for shared resources, shared heaps, and shared fences, by using Name and Access.
//		SetStablePowerState                - A development-time aid for certain types of profiling and experimental prototyping.
// 
//	Device1:
//		CreatePipelineLibrary              - Creates a cached pipeline library.
//		SetEventOnMultipleFenceCompletion  - Specifies an event that should be fired when one or more of a collection of fences reach specific values.
//		SetResidencyPriority               - This method sets residency priorities of a specified list of objects.
// 
//	Device2:
//		CreatePipelineState                - Creates a pipeline state object from a pipeline state stream description.
// 
//	Device3:
//		EnqueueMakeResident                - Asynchronously makes objects resident for the device.
//		OpenExistingHeapFromAddress        - Creates a special-purpose diagnostic heap in system memory from an address. The created heap can persist even in the event of a GPU-fault or device-removed scenario.
//		OpenExistingHeapFromFileMapping    - Creates a special-purpose diagnostic heap in system memory from a file mapping object. The created heap can persist even in the event of a GPU-fault or device-removed scenario.
// 
//	 Device4:
//		CreateCommandList1                 - Creates a command list in the closed state.
//		CreateCommittedResource1           - Creates both a resource and an implicit heap (optionally for a protected session), such that the heap is big enough to contain the entire resource, and the resource is mapped to the heap.
//		CreateHeap1                        - Creates a heap (optionally for a protected session) that can be used with placed resources and reserved resources.
//		CreateProtectedResourceSession     - Creates an object that represents a session for content protection.
//		CreateReservedResource1            - Creates a resource (optionally for a protected session) that is reserved, and not yet mapped to any pages in a heap.
//		GetResourceAllocationInfo1         - Gets rich info about the size and alignment of memory required for a collection of resources on this adapter. (ID3D12Device4::GetResourceAllocationInfo1)
// 
//	Device5:
//		CheckDriverMatchingIdentifier                   - Reports the compatibility of serialized data...
//		CreateLifetimeTracker                           - Creates a lifetime tracker associated with an application-defined callback; the callback receives notifications when the lifetime of a tracked object is changed.
//		CreateMetaCommand                               - Creates an instance of the specified meta command.
//		CreateStateObject                               - Creates an ID3D12StateObject.
//		EnumerateMetaCommandParameters                  - Queries reflection metadata about the parameters of the specified meta command.
//		EnumerateMetaCommands                           - Queries reflection metadata about available meta commands.
//		GetRaytracingAccelerationStructurePrebuildInfo  - Query the driver for resource requirements to build an acceleration structure.
//		RemoveDevice                                    - You can call RemoveDevice to indicate to the Direct3D 12 runtime that the GPU device encountered a problem, and can no longer be used.
// 
//	Device6:
//		SetBackgroundProcessingMode                     - Sets the mode for driver background processing optimizations.
// 
//	Device7:
//		AddToStateObject                                - Incrementally add to an existing state object. This incurs lower CPU overhead than creating a state object from scratch that is a superset of an existing one.
//		CreateProtectedResourceSession1                 - Revises the ID3D12Device4::CreateProtectedResourceSession method with provision GUID that indicates the type of protected resource session.
// 
//	Device8:
//		CreateCommittedResource2                        - Creates both a resource and an implicit heap (optionally for a protected session), such that the heap is big enough to contain the entire resource, and the resource is mapped to the heap.
//		CreatePlacedResource1                           - Creates a resource that is placed in a specific heap. Placed resources are the lightest weight resource objects available, and are the fastest to create and destroy.
//		CreateSamplerFeedbackUnorderedAccessView        - For purposes of sampler feedback, creates a descriptor suitable for binding.
//		GetCopyableFootprints1                          -  Gets a resource layout that can be copied. Helps your app fill in D3D12_PLACED_SUBRESOURCE_FOOTPRINT and D3D12_SUBRESOURCE_FOOTPRINT when suballocating space in upload heaps.
//		GetResourceAllocationInfo2                      - Gets rich info about the size and alignment of memory required for a collection of resources on this adapter. (ID3D12Device8::GetResourceAllocationInfo2)
// 
//	Device9:
//		CreateCommandQueue1                             - Creates a command queue with a creator ID.
//		CreateShaderCacheSession                        - Creates an object that grants access to a shader cache, potentially opening an existing cache or creating a new one.
//		ShaderCacheControl                              - Modifies the behavior of caches used internally by Direct3D or by the driver.
// 
// NOTE: There are more device versions but they're all based on the Agility SDK which allows devs to adopt the newst DirectX 12 graphics features on an older OS.
// TODO: Agility SDK...
using DXDevice = Microsoft::WRL::ComPtr<ID3D12Device>;
using DXDevice1 = Microsoft::WRL::ComPtr<ID3D12Device1>;
using DXDevice2 = Microsoft::WRL::ComPtr<ID3D12Device2>;
using DXDevice3 = Microsoft::WRL::ComPtr<ID3D12Device3>;
using DXDevice4 = Microsoft::WRL::ComPtr<ID3D12Device4>;
using DXDevice5 = Microsoft::WRL::ComPtr<ID3D12Device5>;
using DXDevice6 = Microsoft::WRL::ComPtr<ID3D12Device6>;
using DXDevice7 = Microsoft::WRL::ComPtr<ID3D12Device7>;
using DXDevice8 = Microsoft::WRL::ComPtr<ID3D12Device8>;
using DXDevice9 = Microsoft::WRL::ComPtr<ID3D12Device9>;

// A resource is an object that encapsulates some amount of GPU- accessible memory. A resource exists the same way any object with malloc or virtual alloc exists, meaning 'somewhere' on the heap.
// A resource is therefor just a chuck of bytes but it is meant to be used for textures, surface buffers, vertex buffers, constant buffers. 
// 
// However the resource itself doesn't actually know what it is. It only knows its own layout in memory. Interpertation of that layout
// is done using D3D12_RESOURCE_DESC.
// 
// Since the resource doesn't store the description in itself, the user must handle it. DX12 does provide type for it however which is called a descriptor heap
// which stores all of the descriptors. Remember, a resource may have any layout, but the descriptor struct is always the same.
//
// Supported Methods:
//	Resource:
//		GetDesc - D3D12_RESOURCE_DESC {
//						D3D12_RESOURCE_DIMENSION Dimension;         - One member of D3D12_RESOURCE_DIMENSION, specifying the dimensions of the resource (for example, D3D12_RESOURCE_DIMENSION_TEXTURE1D)
//						UINT64                   Alignment;         - Specifies the alignment.
//						UINT64                   Width;             - Specifies the width of the resource.
//						UINT                     Height;            - Specifies the height of the resource.
//						UINT16                   DepthOrArraySize;  - Specifies the depth of the resource, if it is 3D, or the array size if it is an array of 1D or 2D resources.
//						UINT16                   MipLevels;         - Specifies the number of MIP levels.
//						DXGI_FORMAT              Format;            - Specifies one member of DXGI_FORMAT.
//						DXGI_SAMPLE_DESC         SampleDesc;        - Specifies a DXGI_SAMPLE_DESC structure.
//						D3D12_TEXTURE_LAYOUT     Layout;            - Specifies one member of D3D12_TEXTURE_LAYOUT.
//						D3D12_RESOURCE_FLAGS     Flags;             - Bitwise-OR'd flags, as D3D12_RESOURCE_FLAGS enumeration constants.
//					}
//		GetGPUVirtualAddress   - This method returns the GPU virtual address of a buffer resource.
//		GetHeapProperties      - Retrieves the properties of the resource heap, for placed and committed resources.
//		Map                    - Gets a CPU pointer to the specified subresource in the resource, but may not disclose the pointer value to applications.
//		ReadFromSubresource    - Uses the CPU to copy data from a subresource, enabling the CPU to read the contents of most textures with undefined layouts.
//		Unmap                  - Invalidates the CPU pointer to the specified subresource in the resource.
//		WriteToSubresource     - Uses the CPU to copy data into a subresource, enabling the CPU to modify the contents of most textures with undefined layouts.
// 
//	Resource1:
//		GetProtectedResourceSession - ???
// 
//	Resource2:
//		GetDesc1 - Added: D3D12_MIP_REGION SamplerFeedbackMipRegion; - Describes a mip region with width, height, depth.
using DXResource  = Microsoft::WRL::ComPtr<ID3D12Resource>;
using DXResource1 = Microsoft::WRL::ComPtr<ID3D12Resource1>;
using DXResource2 = Microsoft::WRL::ComPtr<ID3D12Resource2>;

// Since the resrouce is just a chunk of bytes, only knowing its own dimensions, the interpretation of the resource is done using D3D12_RESOURCE_DESC.
// D3D12_RESOURCE_DESC is again external and managing is manual. Descriptor heap is however meant to store all of the various descriptors in the same
// array of memory.
// 
// Keep in mind!!! Indexing is done manually which implies the index must also be tracked. Descriptor heap is just a dumb contiguous memory.
// Tracking the index also enables clean up logic without invalidating the whole system.
// 
// The resource can be abstracted into one level higher object which contains the resource and the index. On top of that a manager that tracks
// free indecies and nexts, similar to how an optimized BVH tree tracks free slots.
//
// Supported Methods:
//	DescriptorHeap:
//		GetCPUDescriptorHandleForHeapStart  - Gets the CPU descriptor handle that represents the start of the heap.
//		GetDesc                             - D3D12_DESCRIPTOR_HEAP_DESC {
// 				                            			D3D12_DESCRIPTOR_HEAP_TYPE  Type;            - A D3D12_DESCRIPTOR_HEAP_TYPE-typed value that specifies the types of descriptors in the heap.
// 				                            			UINT                        NumDescriptors;  - The number of descriptors in the heap.
// 				                            			D3D12_DESCRIPTOR_HEAP_FLAGS Flags;           - A combination of D3D12_DESCRIPTOR_HEAP_FLAGS-typed values that are combined by using a bitwise OR operation. The resulting value specifies options for the heap.
// 				                            			UINT                        NodeMask;        - For single-adapter operation, set this to zero.
//				                            		}
//		GetGPUDescriptorHandleForHeapStart - Gets the GPU descriptor handle that represents the start of the heap.
using DXDescriptorHeap = Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>;

// While the device is used to create all of the necessary objects for graphics programming, all of the real communication between the programmer, CPU and GPU
// is done using the command list.
// 
// It is important to achieve a smooth flow of events without forcing anything to stall. Commands are recorded into a chunck of memory ( command allocator ) by the command list, from the 
// applications logic. The chunck of memory is then passed into the command queue for execution which will run asynchronously to the CPU. This means the list should now be assined
// a new command allocator to record commands into. This avoids any stalling, result a smooth runtime.
// 
// An application can use just one command list and have multiple allocators it swaps between. But in case more lists are required, an allocator should NOT
// have more than one list writing into it.
// 
// Supported Methods:
//	CommandList:
//		GetType - Gets the type of the command list, such as direct, bundle, compute, or copy.
// 
//	GraphicsCommandList:
//		BeginEvent                         - Not intended to be called directly.
//		BeginQuery                         - Starts a query running.
//		ClearDepthStencilView              - Clears the depth-stencil resource.
//		ClearRenderTargetView              - Sets all the elements in a render target to one value.
//		ClearState                         - Resets the state of a direct command list back to the state it was in when the command list was created.
//		ClearUnorderedAccessViewFloat      - Sets all the elements in a unordered access view to the specified float values.
//		ClearUnorderedAccessViewUint       - Sets all the elements in a unordered-access view (UAV) to the specified integer values.
//		Close                              - Indicates that recording to the command list has finished.
//		CopyBufferRegion                   - Copies a region of a buffer from one resource to another.
//		CopyResource                       - Copies the entire contents of the source resource to the destination resource.
//		CopyTextureRegion                  - This method uses the GPU to copy texture data between two locations.
//		CopyTiles                          - Copies tiles from buffer to tiled resource or vice versa.
//		DiscardResource                    - Discards a resource.
//		Dispatch                           - Executes a compute shader on a thread group.
//		DrawIndexedInstanced               - Draws indexed, instanced primitives.
//		DrawInstanced                      - Draws non-indexed, instanced primitives.
//		EndEvent                           - Not intended to be called directly.
//		EndQuery                           - Ends a running query.
//		ExecuteBundle                      - Executes a bundle.
//		ExecuteIndirect                    - Apps perform indirect draws/dispatches using the ExecuteIndirect method.
//		IASetIndexBuffer                   - Sets the view for the index buffer.
//		IASetPrimitiveTopology             - Bind information about the primitive type, and data order that describes input data for the input assembler stage.
//		IASetVertexBuffers                 - Sets a CPU descriptor handle for the vertex buffers.
//		OMSetBlendFactor                   - Sets the blend factor that modulate values for a pixel shader, render target, or both.
//		OMSetRenderTargets                 - Sets CPU descriptor handles for the render targets and depth stencil.
//		OMSetStencilRef                    - Sets the reference value for depth stencil tests.
//		Reset                              - Resets a command list back to its initial state as if a new command list was just created.
//		ResolveQueryData                   - Extracts data from a query. ResolveQueryData works with all heap types (default, upload, and readback).
//		ResolveSubresource                 - Copy a multi-sampled resource into a non-multi-sampled resource.
//		ResourceBarrier                    - Notifies the driver that it needs to synchronize multiple accesses to resources.
//		RSSetScissorRects                  - Binds an array of scissor rectangles to the rasterizer stage.
//		RSSetViewports                     - Bind an array of viewports to the rasterizer stage of the pipeline.
//		SetComputeRoot32BitConstant        - Sets a constant in the compute root signature.
//		SetComputeRoot32BitConstants       - Sets a group of constants in the compute root signature.
//		SetComputeRootConstantBufferView   - Sets a CPU descriptor handle for the constant buffer in the compute root signature.
//		SetComputeRootDescriptorTable      - Sets a descriptor table into the compute root signature.
//		SetComputeRootShaderResourceView   - Sets a CPU descriptor handle for the shader resource in the compute root signature.
//		SetComputeRootSignature            - Sets the layout of the compute root signature.
//		SetComputeRootUnorderedAccessView  - Sets a CPU descriptor handle for the unordered-access-view resource in the compute root signature.
//		SetDescriptorHeaps                 - Changes the currently bound descriptor heaps that are associated with a command list.
//		SetGraphicsRoot32BitConstant       - Sets a constant in the graphics root signature.
//		SetGraphicsRoot32BitConstants      - Sets a group of constants in the graphics root signature.
//		SetGraphicsRootConstantBufferView  - Sets a CPU descriptor handle for the constant buffer in the graphics root signature.
//		SetGraphicsRootDescriptorTable     - Sets a descriptor table into the graphics root signature.
//		SetGraphicsRootShaderResourceView  - Sets a CPU descriptor handle for the shader resource in the graphics root signature.
//		SetGraphicsRootSignature           - Sets the layout of the graphics root signature.
//		SetGraphicsRootUnorderedAccessView - Sets a CPU descriptor handle for the unordered-access-view resource in the graphics root signature.
//		SetMarker                          - Not intended to be called directly.
//		SetPipelineState                   - Sets all shaders and programs most of the fixed-function state of the graphics processing unit (GPU) pipeline.
//		SetPredication                     - Sets a rendering predicate.
//		SOSetTargets                       - Sets the stream output buffer views.
// 
//	GraphicsCommandList1:
//		AtomicCopyBufferUINT               - Atomically copies a primary data element of type UINT from one resource to another, along with optional dependent resources.
//		AtomicCopyBufferUINT64             - Atomically copies a primary data element of type UINT64 from one resource to another, along with optional dependent resources.
//		OMSetDepthBounds                   - This method enables you to change the depth bounds dynamically.
//		ResolveSubresourceRegion           - Copy a region of a multisampled or compressed resource into a non-multisampled or non-compressed resource.
//		SetSamplePositions                 - This method configures the sample positions used by subsequent draw, copy, resolve, and similar operations.
//		SetViewInstanceMask                - Set a mask that controls which view instances are enabled for subsequent draws.
//
//	GraphicsCommandList2:
//		WriteBufferImmediate               - Writes a number of 32-bit immediate values to the specified buffer locations directly from the command stream.
// 
//	GraphicsCommandList3:
//		SetProtectedResourceSession        - Specifies whether or not protected resources can be accessed by subsequent commands in the command list.
// 
//	GraphicsCommandList4:
//		BeginRenderPass                                  - Marks the beginning of a render pass by binding a set of output resources for the duration of the render pass.
//		BuildRaytracingAccelerationStructure             - Performs a raytracing acceleration structure build on the GPU and optionally outputs post-build information immediately after the build.
//		CopyRaytracingAccelerationStructure              - Copies a source acceleration structure to destination memory while applying the specified transformation.
//		DispatchRays                                     - Launch the threads of a ray generation shader.
//		EmitRaytracingAccelerationStructurePostbuildInfo - Emits post-build properties for a set of acceleration structures. This enables applications to know the output resource requirements for performing acceleration structure operations.
//		EndRenderPass                                    - Marks the ending of a render pass.
//		ExecuteMetaCommand                               - Records the execution (or invocation) of the specified meta command into a graphics command list.
//		InitializeMetaCommand                            - Initializes the specified meta command.
//		SetPipelineState1                                - Sets a state object on the command list.
// 
//	GraphicsCommandList5:
//		RSSetShadingRate                                 - Sets the base shading rate, and combiners, for variable-rate shading (VRS).
//		RSSetShadingRateImage                            - sets the screen-space shading-rate image for variable-rate shading (VRS).
//	
//	GraphicsCommandList6:
//		DispatchMesh                                     - dispatches mesh supposedly, no descrip...
// 
//	GraphicsCommandList7:
//		Barrier                                          - Adds a collection of barriers into a graphics command list recording.
// 
// NOTE: There are more through 10 but they don't have official documentation on msdn. Can be found on github.
// 
//	GraphicsCommandList8:
//		OMSetFrontAndBackStencilRef      - ???
// 
//	GraphicsCommandList9:
//		RSSetDepthBias                   - ???
//		IASetIndexBufferStripCutValue    - ???
// 
//	GraphicsCommandList10:
//		SetProgram                       - ???
//		DispatchGraph                    - ???
using DXCommandList = Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>;
using DXCommandList1 = Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList1>;
using DXCommandList2 = Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>;
using DXCommandList3 = Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList3>;
using DXCommandList4 = Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>;
using DXCommandList5 = Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5>;
using DXCommandList6 = Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6>;
using DXCommandList7 = Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7>;

// The command allocator is the backing memory of the command list. The reason why the allocator and list exist separately is because
// of synchronization responsibility between the GPU and CPU. Once the list passes the commands to the command queue for execution, the list
// may be reused using a different allocator while the command queue reads from the previous allocator. The command allocator which is currently being
// used by the command queue may not be reset, written into or moved until the command queue is done.
// There must be at least one command allocator per back buffer resource.
// 
// Supported Methods:
//	CommandAllocator:
//		Reset - Indicates to re-use the memory that is associated with the command allocator.
using DXCommandAllocator = Microsoft::WRL::ComPtr<ID3D12CommandAllocator>;

// The command queue provides methods for submitting command lists, synchronizing command list execution, instrumenting the command queue, and updating resource tile mappings.
// Simply put the command queue accepts a command allocator from the command list and starts executing into the resource managed by the swap chain.
// The command allocators which are used by the command queue may not be reset until the command queue signals 'done'.
// 
// The command queue is created using D3D12Device.
// A form must be filled to create a command queue:
// 
// D3D12_COMMAND_QUEUE_DESC {
//				D3D12_COMMAND_LIST_TYPE   Type;     - Specifies one member of D3D12_COMMAND_LIST_TYPE.
//				INT                       Priority; - The priority for the command queue, as a D3D12_COMMAND_QUEUE_PRIORITY enumeration constant to select normal or high priority.
//				D3D12_COMMAND_QUEUE_FLAGS Flags;    - Specifies any flags from the D3D12_COMMAND_QUEUE_FLAGS enumeration.
//				UINT                      NodeMask; - For single GPU operation, set this to zero. If there are multiple GPU nodes, set a bit to identify the node (the device's physical adapter) to which the command queue applies. 
//			}
//
// 
// Supported Methods:
//		BeginEvent            - Not intended to be called directly.  Use the PIX event runtime to insert events into a command queue.
//		CopyTileMappings      - Copies mappings from a source reserved resource to a destination reserved resource.
//		EndEvent              - Not intended to be called directly.  Use the PIX event runtime to insert events into a command queue.
//		ExecuteCommandLists   - Submits an array of command lists for execution.
//		GetClockCalibration   - This method samples the CPU and GPU timestamp counters at the same moment in time.
//		GetDesc               - Gets the description of the command queue.
//		GetTimestampFrequency - This method is used to determine the rate at which the GPU timestamp counter increments.
//		SetMarker             - Not intended to be called directly.  Use the PIX event runtime to insert events into a command queue. 
//		Signal                - Updates a fence to a specified value.
//		UpdateTileMappings    - Updates mappings of tile locations in reserved resources to memory locations in a resource heap.
//		Wait                  - Queues a GPU-side wait, and returns immediately. A GPU-side wait is where the GPU waits until the specified fence reaches or exceeds the specified value.
using DXCommandQueue = Microsoft::WRL::ComPtr<ID3D12CommandQueue>;

// The command allocators are just heaps of memory. Command lists write in those heaps of memory. The command queue will then execute a piece of memory.
// On the CPU side, the game logic side, you do not want to stall the whole system and wait until the GPU is done. So you create multiple allocators and swap between them.
// Frame 1 the list fills allocator1 and passes, frame 2 the list fills 2 and passes, the command queue is done with frame 1 in frame 2, but it might not. The list needs to be 
// assigned allocator1 after it passes allocator2, otherwise it has nothing to record commands into, but you can't write into an allocator which may still be read from by the GPU.
// 
// The CPU and GPU run on different timelines.
// 
// To synchronize the system, and thus avoiding fatal errors, a fence is used. A ID3D12Fence acts as an integer and a view for both the CPU and GPU. An intermediary of sorts.
// When the list passes an allocator to the GPU it also says when it expects the value to be back. Once the GPU is done it writes that value and the cycle can repeat.
// 
// Supported Methods:
//	Fence:
//		GetCompletedValue       - Gets the current value of the fence.
//		SetEventOnCompletion    - Specifies an event that should be fired when the fence reaches a certain value.
//		Signal                  - Sets the fence to the specified value.
using DXFence = Microsoft::WRL::ComPtr<ID3D12Fence>;

// The swap chain implements one or more surfaces for storing rendered data before presenting it to an output.
// Simply, resources are created and their role is to act as buffers that get rendered into. The swap chains role is that of a manager. It says which resource may be rendered into,
// which is ready to be presented. To surfaces are written into by the command queue executing commands into it.
// 
// A swap chain should always juggle at least 2 resources, ideally 3 for a smooth animation, avoiding tearing and other potential artifacts. More than 3 is not better.
// Minimal swap chain for dx12 is SwapChain3.
//
// Supported Methods:
// 
//	SwapChain:
//		GetBuffer               - Accesses one of the swap-chain's back buffers.
//		GetContainingOutput     - Get the output (the display monitor) that contains the majority of the client area of the target window.
//		GetDesc                 - Get a description of the swap chain. ( minimal swap chain version in dx12 should be 3 so this version is not useful. )
//		GetFrameStatistics      - Gets performance statistics about the last render frame.
//		GetFullscreenState      - Get the state associated with full-screen mode.
//		GetLastPresentCount     - Gets the number of times that IDXGISwapChain::Present or IDXGISwapChain1::Present1 has been called.
//		Present                 - Presents a rendered image to the user.
//		ResizeBuffers           - Changes the swap chain's back buffer size, format, and number of buffers. This should be called when the application window is resized.
//		ResizeTarget            - Resizes the output target.
//		SetFullscreenState      - Sets the display state to windowed or full screen.
// 
//	SwapChain1:
//		GetBackgroundColor      - Retrieves the background color of the swap chain.
//		GetCoreWindow           - Retrieves the underlying CoreWindow object for this swap-chain object.
//		GetDesc1                - DXGI_SWAP_CHAIN_DESC1 {
//				                			UINT             Width;       - A value that describes the resolution width. If 0 then it will be set to the containment windows width.
//				                			UINT             Height;      - A value that describes the resolution height. If 0 then it will be set to the containment windows height.
//				                			DXGI_FORMAT      Format;      - A DXGI_FORMAT structure that describes the display format.
//				                			BOOL             Stereo;      - Specifies whether the full-screen display mode or the swap-chain back buffer is stereo.
//				                			DXGI_SAMPLE_DESC SampleDesc;  - A DXGI_SAMPLE_DESC structure that describes multi-sampling parameters.
//				                			DXGI_USAGE       BufferUsage; - A DXGI_USAGE-typed value that describes the surface usage and CPU access options for the back buffer.
//				                			UINT             BufferCount; - A value that describes the number of buffers in the swap chain. 
//				                			DXGI_SCALING     Scaling;     - A DXGI_SCALING-typed value that identifies resize behavior if the size of the back buffer is not equal to the target output.
//				                			DXGI_SWAP_EFFECT SwapEffect;  - A DXGI_SWAP_EFFECT-typed value that describes the presentation model that is used by the swap chain and options for handling the contents of the presentation buffer after presenting a surface. 
//				                			DXGI_ALPHA_MODE  AlphaMode;   - A DXGI_ALPHA_MODE-typed value that identifies the transparency behavior of the swap-chain back buffer.
//				                			UINT             Flags;       - Value specifies options for swap-chain behavior.
//				                 		}
//		GetFullscreenDesc        - Gets a description of a full-screen swap chain.
//		GetHwnd                  - Retrieves the underlying HWND for this swap-chain object.
//		GetRestrictToOutput      - Gets the output (the display monitor) to which you can restrict the contents of a present operation.
//		GetRotation              - Gets the rotation of the back buffers for the swap chain.
//		IsTemporaryMonoSupported - Determines whether a swap chain supports “temporary mono.”
//		Present1                 - Presents a frame on the display screen. An app can use Present1 to optimize presentation by specifying scroll and dirty rectangles.
//		SetBackgroundColor       - Changes the background color of the swap chain.
//		SetRotation              - Sets the rotation of the back buffers for the swap chain.
//
//	SwapChain2:
//		GetFrameLatencyWaitableObject - Returns a waitable handle that signals when the DXGI adapter has finished presenting a new frame. In dx12 this type of task is delegated to the fence. Not usable.
//		GetMatrixTransform            - Gets the transform matrix that will be applied to a composition swap chain upon the next present.
//		GetMaximumFrameLatency        - Gets the number of frames that the swap chain is allowed to queue for rendering.
//		GetSourceSize                 - Gets the source region used for the swap chain.
//		SetMatrixTransform            - Sets the transform matrix that will be applied to a composition swap chain upon the next present.
//		SetMaximumFrameLatency        - Sets the number of frames that the swap chain is allowed to queue for rendering.
//		SetSourceSize                 - Sets the source region to be used for the swap chain.
// 
//	SwapChain3:
//		CheckColorSpaceSupport        - Checks the swap chain's support for color space.
//		GetCurrentBackBufferIndex     - Gets the index of the swap chain's current back buffer.
//		ResizeBuffers1                - Changes the swap chain's back buffer size, format, and number of buffers, where the swap chain was created using a D3D12 command queue as an input device. This should be called when the application window is resized.
//		SetColorSpace1                - Sets the color space used by the swap chain. (IDXGISwapChain3.SetColorSpace1)
//
//	SwapChain4:
//		SetHDRMetaData                - This method sets High Dynamic Range (HDR) and Wide Color Gamut (WCG) header metadata.
using DXSwapChain1 = Microsoft::WRL::ComPtr<IDXGISwapChain1>;
using DXSwapChain3 = Microsoft::WRL::ComPtr<IDXGISwapChain3>;
using DXSwapChain4 = Microsoft::WRL::ComPtr<IDXGISwapChain4>;


// ###################################################################################################################################
// ###################################################################################################################################
// ###################################################################################################################################
// ###################################################################################################################################
// ###################################################################################################################################

// InfoQueue is an information queue interface in directX which stores, retrieves, and filters debug messages. The queue consists of a message
// queue, an optional storage filter stack, and a optional retrieval filter stack.
// 
// The interface is obtained by querying it from ID3D12Device using IUnknown::QueryInterface.
// 
// The ID3D12Debug layer must be enabled through ID3D12Debug::EnableDebugLayer for that operation to succeed.
// 
// Supported Methods:
//	InfoQueue:
//		AddApplicationMessage                        - Adds a user-defined message to the message queue and sends that message to debug output.
//		AddMessage                                   - Adds a debug message to the message queue and sends that message to debug output.
//		AddRetrievalFilterEntries                    - Add storage filters to the top of the retrieval-filter stack.
//		AddStorageFilterEntries                      - Add storage filters to the top of the storage-filter stack.
//		ClearRetrievalFilter                         - Remove a retrieval filter from the top of the retrieval-filter stack.
//		ClearStorageFilter                           - Remove a storage filter from the top of the storage-filter stack.
//		ClearStoredMessages                          - Clear all messages from the message queue.
//		GetBreakOnCategory                           - Get a message category to break on when a message with that category passes through the storage filter.
//		GetBreakOnID                                 - Get a message identifier to break on when a message with that identifier passes through the storage filter.
//		GetBreakOnSeverity                           - Get a message severity level to break on when a message with that severity level passes through the storage filter.
//		GetMessage                                   - Get a message from the message queue.
//		GetMessageCountLimit                         - Get the maximum number of messages that can be added to the message queue.
//		GetMuteDebugOutput                           - Get a boolean that determines if debug output is on or off.
//		GetNumMessagesAllowedByStorageFilter         - Get the number of messages that were allowed to pass through a storage filter.
//		GetNumMessagesDeniedByStorageFilter          - Get the number of messages that were denied passage through a storage filter.
//		GetNumMessagesDiscardedByMessageCountLimit   - Get the number of messages that were discarded due to the message count limit.
//		GetNumStoredMessages                         - Get the number of messages currently stored in the message queue.
//		GetNumStoredMessagesAllowedByRetrievalFilter - Get the number of messages that are able to pass through a retrieval filter.
//		GetRetrievalFilter                           - Get the retrieval filter at the top of the retrieval-filter stack.
//		GetRetrievalFilterStackSize                  - Get the size of the retrieval-filter stack in bytes.
//		GetStorageFilter                             - Get the storage filter at the top of the storage-filter stack.
//		GetStorageFilterStackSize                    - Get the size of the storage-filter stack in bytes.
//		PopRetrievalFilter                           - Pop a retrieval filter from the top of the retrieval-filter stack.
//		PopStorageFilter                             - Pop a storage filter from the top of the storage-filter stack.
//		PushCopyOfRetrievalFilter                    - Push a copy of retrieval filter currently on the top of the retrieval-filter stack onto the retrieval-filter stack.
//		PushCopyOfStorageFilter                      - Push a copy of storage filter currently on the top of the storage-filter stack onto the storage-filter stack.
//		PushEmptyRetrievalFilter                     - Push an empty retrieval filter onto the retrieval-filter stack.
//		PushEmptyStorageFilter                       - Push an empty storage filter onto the storage-filter stack.
//		PushRetrievalFilter                          - Push a retrieval filter onto the retrieval-filter stack.
//		PushStorageFilter                            - Push a storage filter onto the storage-filter stack.
//		SetBreakOnCategory                           - Set a message category to break on when a message with that category passes through the storage filter.
//		SetBreakOnID                                 - Set a message identifier to break on when a message with that identifier passes through the storage filter.
//		SetBreakOnSeverity                           - Set a message severity level to break on when a message with that severity level passes through the storage filter.
//		SetMessageCountLimit                         - Set the maximum number of messages that can be added to the message queue.
//		SetMuteDebugOutput                           - Set a boolean that turns the debug output on or off.
using DXInfoQueue = Microsoft::WRL::ComPtr<ID3D12InfoQueue>;


HRESULT find_adapter(DXFactory4& factory, bool use_warp, DXAdapter4& adapter_out);

// in release mode this function always returns S_OK
HRESULT enable_GPU_debug_layer();

bool check_is_tearing_supported(DXFactory4& factory4);