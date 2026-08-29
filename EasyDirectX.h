#pragma once

#include "badDirectX.h"

// pipeline state object defs

template <typename T, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE TypeValue>
struct PIPELINE_STATE_STREAM_OBJECT
{
	PIPELINE_STATE_STREAM_OBJECT() = default;

	PIPELINE_STATE_STREAM_OBJECT(const T& value)
		: object(value)
	{
	}

	PIPELINE_STATE_STREAM_OBJECT& operator=(const T& value)
	{
		object = value;
		return *this;
	}

	D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type = TypeValue;
	T object{};
};

using PSS_ROOT_SIGNATURE     = PIPELINE_STATE_STREAM_OBJECT<ID3D12RootSignature*,          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE>;
using PSS_INPUT_LAYOUT       = PIPELINE_STATE_STREAM_OBJECT<D3D12_INPUT_LAYOUT_DESC,       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT>;
using PSS_PRIMITIVE_TOPOLOGY = PIPELINE_STATE_STREAM_OBJECT<D3D12_PRIMITIVE_TOPOLOGY_TYPE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY>;
using PSS_VERTEX_SHADER      = PIPELINE_STATE_STREAM_OBJECT<D3D12_SHADER_BYTECODE,         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS>;
using PSS_PIXEL_SHADER       = PIPELINE_STATE_STREAM_OBJECT<D3D12_SHADER_BYTECODE,         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS>;
using PSS_DSV_FORMAT         = PIPELINE_STATE_STREAM_OBJECT<DXGI_FORMAT,                   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT>;
using PSS_RTV_FORMATS        = PIPELINE_STATE_STREAM_OBJECT<D3D12_RT_FORMAT_ARRAY,         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS>;

// barriers
struct Barriers 
{
	static constexpr D3D12_RESOURCE_BARRIER transition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) noexcept
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = before;
		barrier.Transition.StateAfter = after;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		return barrier;
	}
};

// heap descs
struct HeapDesc 
{
	static constexpr D3D12_DESCRIPTOR_HEAP_DESC custom(UINT desc_count, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, UINT node_masks = 0) noexcept
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = desc_count;
		desc.Type = type;
		desc.NodeMask = node_masks;
		desc.Flags = flags;

		return desc;
	}

	static constexpr D3D12_DESCRIPTOR_HEAP_DESC RTV(UINT desc_count) noexcept
	{
		return custom(desc_count, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
};
