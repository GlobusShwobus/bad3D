#pragma once

#include <d3d12.h>
#include <wrl/client.h>

class Resource
{
public:
    Resource() = default;
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource(Resource&&) = default;
    Resource& operator=(Resource&&) = default;

    Resource(
        ID3D12Device4* device,
        const D3D12_HEAP_PROPERTIES& heap_properties,
        const D3D12_RESOURCE_DESC& resource_desc,
        D3D12_RESOURCE_STATES initial_state,
        D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE,
        const D3D12_CLEAR_VALUE* optimized_clear_value = nullptr);

    ID3D12Resource* get() const noexcept { return mResource.Get(); }
    D3D12_RESOURCE_DESC desc() const;

    D3D12_RESOURCE_STATES exchange_state(D3D12_RESOURCE_STATES after) noexcept; // something something resource transitions should be done in bulk? this helps but also means it must be switched back later
    void transition_state(ID3D12GraphicsCommandList2* list, D3D12_RESOURCE_STATES after, D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, UINT sub_resource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mResource = nullptr;
    D3D12_RESOURCE_STATES mState = D3D12_RESOURCE_STATE_COMMON;
};

class VertexBuffer
{
public:
    VertexBuffer() = default;
    VertexBuffer(ID3D12Device4* device, UINT64 byte_size, UINT element_type_size);

    ID3D12Resource* get() const noexcept { return mResource.get(); }
    Resource& resource() noexcept { return mResource; }

    constexpr UINT stride_in_bytes() const noexcept { return mView.StrideInBytes; }
    constexpr UINT size_in_bytes() const noexcept { return mView.SizeInBytes; }
    constexpr UINT element_count() const noexcept { return mView.SizeInBytes / mView.StrideInBytes; }

    const D3D12_VERTEX_BUFFER_VIEW& view() const noexcept { return mView; }
    D3D12_VERTEX_BUFFER_VIEW create_subview(UINT64 byte_position, UINT byte_count) const noexcept;
private:
    Resource mResource;
    D3D12_VERTEX_BUFFER_VIEW mView = {};
};

class IndexBuffer
{
public:
    IndexBuffer() = default;
    IndexBuffer(ID3D12Device4* device, UINT64 byte_size, DXGI_FORMAT format);

    ID3D12Resource* get() const noexcept { return mResource.get(); }
    Resource& resource() noexcept { return mResource; }

    static constexpr UINT index_stride_from_format(DXGI_FORMAT format) noexcept
    {
        return format == DXGI_FORMAT_R16_UINT ? 2u : 4u;
    }

    constexpr DXGI_FORMAT format() const noexcept { return mView.Format; }
    constexpr UINT size_in_bytes() const noexcept { return mView.SizeInBytes; }
    constexpr UINT element_count() const noexcept {
        return mView.SizeInBytes / index_stride_from_format(mView.Format);
    }

    const D3D12_INDEX_BUFFER_VIEW& view() const noexcept { return mView; }
    D3D12_INDEX_BUFFER_VIEW create_subview(UINT64 byte_position, UINT byte_count) const noexcept;

private:
    Resource mResource;
    D3D12_INDEX_BUFFER_VIEW mView = {};
};