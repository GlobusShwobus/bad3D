#pragma once

#include <d3d12.h>
#include <wrl/client.h>


#include <assert.h>
#include <utility>
class GPUHeap
{
public:
    GPUHeap() = default;
    GPUHeap(ID3D12Device4* device, const D3D12_HEAP_DESC& desc)
    {
        assert(device && "nullptr");

        device->CreateHeap(&desc, IID_PPV_ARGS(&mHeap));

        mDesc = desc;
    }

    GPUHeap(const GPUHeap&) = delete;
    GPUHeap& operator=(const GPUHeap&) = delete;
    GPUHeap(GPUHeap&&) = default;
    GPUHeap& operator=(GPUHeap&&) = default;
    ~GPUHeap() = default;

    constexpr const D3D12_HEAP_DESC& get_desc() const noexcept { return mDesc; }
    ID3D12Heap* get_heap() const noexcept { return mHeap.Get(); }

private:

    Microsoft::WRL::ComPtr<ID3D12Heap> mHeap = nullptr;
    D3D12_HEAP_DESC mDesc = {};
};


class Resource
{
public:
    Resource() = default;
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource(Resource&&) = default;
    Resource& operator=(Resource&&) = default;

    static Resource create_commited(
        ID3D12Device4* device,
        const D3D12_HEAP_PROPERTIES& heap_properties,
        const D3D12_RESOURCE_DESC& resource_desc,
        D3D12_RESOURCE_STATES initial_state,
        D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE,
        const D3D12_CLEAR_VALUE* optimized_clear_value = nullptr
    )
    {
        assert(device && "nullptr");
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        device->CreateCommittedResource(
            &heap_properties,
            flags,
            &resource_desc,
            initial_state,
            optimized_clear_value,
            IID_PPV_ARGS(&resource)
        );
        return Resource(std::move(resource), resource_desc, initial_state);
    }

    static Resource create_placed(
        ID3D12Device4* device,
        GPUHeap& heap,
        UINT64 heap_offset,
        const D3D12_RESOURCE_DESC& desc,
        D3D12_RESOURCE_STATES initial_state,
        const D3D12_CLEAR_VALUE* optimized_clear_value = nullptr)
    {
        assert(device && "nullptr");
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        device->CreatePlacedResource(
            heap.get_heap(),
            heap_offset,
            &desc,
            initial_state,
            optimized_clear_value,
            IID_PPV_ARGS(&resource)
        );
        return Resource(std::move(resource), desc, initial_state);
    }

    ID3D12Resource* get() const noexcept { return mResource.Get(); }
    const D3D12_RESOURCE_DESC& desc() const noexcept { return mDesc; }
    D3D12_RESOURCE_STATES state() const noexcept { return mState; }


    //  void transition_state(ID3D12GraphicsCommandList2* list, D3D12_RESOURCE_STATES after, UINT sub_resource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
    //  {
    //      if (mState == after) return;
    //      list->ResourceBarrier( ... );
    //      mState = after;
    //  }

protected:

    Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource,
        const D3D12_RESOURCE_DESC& desc,
        D3D12_RESOURCE_STATES state) noexcept
        : mResource(std::move(resource)), mDesc(desc), mState(state) 
    {
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
    D3D12_RESOURCE_DESC mDesc;
    D3D12_RESOURCE_STATES mState;
};

class VertexBuffer
{
public:
    VertexBuffer() = default;
    VertexBuffer(Resource resource, UINT64 count, UINT64 stride) noexcept
        : mResource(std::move(resource)), mCount(count), mStride(stride) {
    }

    ID3D12Resource* get() const noexcept { return mResource.get(); }
    Resource& resource() noexcept { return mResource; }

    constexpr UINT64 count()  const noexcept { return mCount; }
    constexpr UINT64 stride() const noexcept { return mStride; }

    D3D12_VERTEX_BUFFER_VIEW view() const noexcept
    {
        return {
            mResource.get()->GetGPUVirtualAddress(),
            static_cast<UINT>(mCount * mStride),
            static_cast<UINT>(mStride)
        };
    }

private:
    Resource mResource;
    UINT64 mCount = 0;
    UINT64 mStride = 0;
};

class IndexBuffer
{
public:
    IndexBuffer() = default;
    IndexBuffer(Resource resource, UINT64 count, DXGI_FORMAT format) noexcept
        : mResource(std::move(resource)), mCount(count), mFormat(format) {
    }

    ID3D12Resource* get() const noexcept { return mResource.get(); }
    Resource& resource() noexcept { return mResource; }

    constexpr UINT64      count()  const noexcept { return mCount; }
    constexpr DXGI_FORMAT format() const noexcept { return mFormat; }

    D3D12_INDEX_BUFFER_VIEW view() const noexcept
    {
        const UINT stride = (mFormat == DXGI_FORMAT_R32_UINT) ? 4u : 2u;
        return {
            mResource.get()->GetGPUVirtualAddress(),
            static_cast<UINT>(mCount * stride),
            mFormat
        };
    }

private:
    Resource mResource;
    UINT64 mCount = 0;
    DXGI_FORMAT mFormat = DXGI_FORMAT_R16_UINT;
};

//  class CommittedResource
//  {
//  public:
//      //TODO: constructors
//      CommittedResource() = default;
//  
//      Microsoft::WRL::ComPtr<ID3D12Resource> load(
//          ID3D12Device4* device,
//          ID3D12GraphicsCommandList2* cl,
//          const void* data, UINT64 element_count, UINT64 type_size);
//  
//  
//      ID3D12Resource* get() const noexcept;
//  
//      constexpr UINT64 count() const noexcept
//      {
//          return mCount;
//      }
//  
//      constexpr UINT64 stride() const noexcept
//      {
//          return mStride;
//      }
//  
//  private:
//      Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
//      UINT64                            mCount = 0;
//      UINT64                            mStride = 0;
//  };