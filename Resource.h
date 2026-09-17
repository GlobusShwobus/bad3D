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

    static Resource create_commited(
        ID3D12Device4* device,
        const D3D12_HEAP_PROPERTIES& heap_properties,
        const D3D12_RESOURCE_DESC& resource_desc,
        D3D12_RESOURCE_STATES initial_state,
        D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE,
        const D3D12_CLEAR_VALUE* optimized_clear_value = nullptr);

    ID3D12Resource* get() const noexcept { return mResource.Get(); }
    const D3D12_RESOURCE_DESC& desc() const noexcept { return mDesc; }

    D3D12_RESOURCE_STATES exchange_state(D3D12_RESOURCE_STATES after) noexcept; // something something resource transitions should be done in bulk? this helps but also means it must be switched back later
    void transition_state(ID3D12GraphicsCommandList2* list, D3D12_RESOURCE_STATES after, D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, UINT sub_resource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

protected:

    Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource,
        const D3D12_RESOURCE_DESC& desc,
        D3D12_RESOURCE_STATES state) noexcept;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
    D3D12_RESOURCE_DESC mDesc = {};
    D3D12_RESOURCE_STATES mState = D3D12_RESOURCE_STATE_COMMON;
};

class VertexBuffer
{
public:
    VertexBuffer() = default;
    VertexBuffer(Resource resource, UINT element_type_size) noexcept;

    ID3D12Resource* get() const noexcept { return mResource.get(); }
    Resource& resource() noexcept { return mResource; }

    constexpr UINT element_type_size() const noexcept { return mView.StrideInBytes; }
    constexpr UINT element_count() const noexcept  { return mView.SizeInBytes / mView.StrideInBytes; }

    const D3D12_VERTEX_BUFFER_VIEW& view() const noexcept { return mView; }
    D3D12_VERTEX_BUFFER_VIEW create_subview(UINT byte_position, UINT byte_count) const noexcept
    {
        D3D12_VERTEX_BUFFER_VIEW subview = {};

        // i need to check if element_offset*mstride + mstride * element count is less than the the entire sizeinbytes

        if ((byte_position + byte_count) <= mView.SizeInBytes)
        {
            subview.BufferLocation = mView.BufferLocation + byte_position;
            subview.SizeInBytes = byte_count;
            subview.StrideInBytes = mView.StrideInBytes;
        }

        return subview;
    }
private:
    Resource mResource;
    D3D12_VERTEX_BUFFER_VIEW mView = {};
};

class IndexBuffer
{
public:
    IndexBuffer() = default;
    IndexBuffer(Resource resource, DXGI_FORMAT format) noexcept;

    ID3D12Resource* get() const noexcept { return mResource.get(); }
    Resource& resource() noexcept { return mResource; }

    constexpr UINT count() const noexcept
    {
        const UINT stride = mView.Format == DXGI_FORMAT_R16_UINT ? 2u : 4u;
        return mView.SizeInBytes / stride;
    }
    constexpr DXGI_FORMAT format() const noexcept { return mView.Format; }

    const D3D12_INDEX_BUFFER_VIEW& view() const noexcept { return mView; }

    D3D12_INDEX_BUFFER_VIEW create_subview(UINT byte_position, UINT byte_count) const noexcept
    {
        D3D12_INDEX_BUFFER_VIEW subview = {};

        // i need to check if element_offset*mstride + mstride * element count is less than the the entire sizeinbytes
        const UINT per_element_stride = mView.Format == DXGI_FORMAT_R16_UINT ? 2u : 4u;

        if ((byte_position + byte_count) <= mView.SizeInBytes)
        {
            subview.BufferLocation = mView.BufferLocation + byte_position;
            subview.SizeInBytes = byte_count;
            subview.Format = mView.Format;
        }

        return subview;
    }

private:
    Resource mResource;
    D3D12_INDEX_BUFFER_VIEW mView = {};
};