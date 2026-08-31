#pragma once

#include "badDirectX.h"
#include <concepts>

// TODO: consistency... go over what should have templates, what doesnt need them, what needs custom(...) etc...

// input elements
// - what data comes from c++ side vertex buffers, into hlsl side vertex shader
// - a verbose explicit way of describing a struct that exists in c++ to shader HLSL

struct INPUT_ELEMENT
{
	// semantic index is if struct has more than 1 "POSITION" (or any same) semantic
	// input slot identifies which buffer this element comes from ( in case of struct of arrays )

	static constexpr D3D12_INPUT_ELEMENT_DESC custom_PV(LPCSTR semantic_name, UINT semantic_index = 0, UINT input_slot = 0) noexcept
	{
		D3D12_INPUT_ELEMENT_DESC desc = {};

		desc.SemanticName = semantic_name;
		desc.SemanticIndex = semantic_index;
		desc.InputSlot = input_slot;
		desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		desc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		desc.InstanceDataStepRate = 0;

		return desc;
	}

	static constexpr D3D12_INPUT_ELEMENT_DESC position_PV(UINT semantic_index = 0, UINT input_slot = 0) noexcept
	{
		return custom_PV("POSITION", semantic_index, input_slot);
	}

	static constexpr D3D12_INPUT_ELEMENT_DESC color_PV(UINT semantic_index = 0, UINT input_slot = 0) noexcept
	{
		return custom_PV("COLOR", semantic_index, input_slot);
	}
};

// Input elements is for input assembly - what data i have in code and creating contracts that must match the shader in hlsl
// root signature is similar however instead of what gets passed into hlsl main(...), root signature is responsible for the register data b,t,s...

struct ROOT_PARAMETER
{
	struct DESCRIPTOR_RANGE
	{
		static constexpr D3D12_DESCRIPTOR_RANGE1 custom(
			D3D12_DESCRIPTOR_RANGE_TYPE type,
			UINT                        count,
			UINT                        shader_register,
			UINT                        register_space = 0,
			UINT                        offset = 0,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
		{
			D3D12_DESCRIPTOR_RANGE1 desc{};

			desc.RangeType = type;
			desc.NumDescriptors = count;
			desc.BaseShaderRegister = shader_register;
			desc.RegisterSpace = register_space;
			desc.OffsetInDescriptorsFromTableStart = offset;
			desc.Flags = flags;

			return desc;
		}

		static constexpr D3D12_DESCRIPTOR_RANGE1 SRV(UINT count, UINT shader_register, UINT register_space = 0, UINT offset = 0, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
		{
			return custom(
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				count,
				shader_register,
				register_space,
				offset,
				flags
			);
		}

		static constexpr D3D12_DESCRIPTOR_RANGE1 CBV(UINT count, UINT shader_register, UINT register_space = 0, UINT offset = 0, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
		{
			return custom(
				D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
				count,
				shader_register,
				register_space,
				offset,
				flags
			);
		}

		static constexpr D3D12_DESCRIPTOR_RANGE1 UAV(UINT count, UINT shader_register, UINT register_space = 0, UINT offset = 0, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
		{
			return custom(
				D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
				count,
				shader_register,
				register_space,
				offset,
				flags
			);
		}

		static constexpr D3D12_DESCRIPTOR_RANGE1 sampler(UINT count, UINT shader_register, UINT register_space = 0, UINT offset = 0, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
		{
			return custom(
				D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
				count,
				shader_register,
				register_space,
				offset,
				flags
			);
		}
	};

	struct DESCRIPTOR
	{
		static constexpr D3D12_ROOT_DESCRIPTOR1 descriptor(UINT shader_register, UINT register_space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
		{

			D3D12_ROOT_DESCRIPTOR1 desc{};

			desc.ShaderRegister = shader_register;
			desc.RegisterSpace = register_space;
			desc.Flags = flags;

			return desc;
		}
	};

	struct CONSTANT
	{
		static constexpr D3D12_ROOT_CONSTANTS constant(UINT shader_register, UINT count, UINT register_space = 0) noexcept
		{
			D3D12_ROOT_CONSTANTS desc{};

			desc.ShaderRegister = shader_register;
			desc.Num32BitValues = count;
			desc.RegisterSpace = register_space;

			return desc;
		}
	};

	static constexpr D3D12_ROOT_PARAMETER1 constant(D3D12_SHADER_VISIBILITY visibility, UINT shader_register, UINT count, UINT register_space = 0) noexcept
	{
		D3D12_ROOT_PARAMETER1 desc{};

		desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		desc.ShaderVisibility = visibility;
		desc.Constants = CONSTANT::constant(shader_register, count, register_space);

		return desc;
	}

	static constexpr D3D12_ROOT_PARAMETER1 descriptor(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility, UINT shader_register, UINT register_space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
	{
		D3D12_ROOT_PARAMETER1 desc{};

		desc.ParameterType = type;
		desc.ShaderVisibility = visibility;
		desc.Descriptor = DESCRIPTOR::descriptor(shader_register, register_space, flags);

		return desc;
	}

	static constexpr D3D12_ROOT_PARAMETER1 descriptor_CBV(D3D12_SHADER_VISIBILITY visibility, UINT shader_register, UINT register_space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
	{
		return descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, visibility, shader_register, register_space, flags);
	}

	static constexpr D3D12_ROOT_PARAMETER1 descriptor_SRV(D3D12_SHADER_VISIBILITY visibility, UINT shader_register, UINT register_space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
	{
		return descriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, visibility, shader_register, register_space, flags);
	}

	static constexpr D3D12_ROOT_PARAMETER1 descriptor_UAV(D3D12_SHADER_VISIBILITY visibility, UINT shader_register, UINT register_space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
	{
		return descriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, visibility, shader_register, register_space, flags);
	}

	static constexpr D3D12_ROOT_PARAMETER1 descriptor_table(D3D12_SHADER_VISIBILITY visibility, UINT range_count, const D3D12_DESCRIPTOR_RANGE1* range) noexcept
	{
		D3D12_ROOT_PARAMETER1 desc{};

		desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.ShaderVisibility = visibility;
		desc.DescriptorTable.NumDescriptorRanges = range_count;
		desc.DescriptorTable.pDescriptorRanges = range;

		return desc;
	}
};

// what shader stage root sig is visible in, like a global (different from D3D12_SHADER_VISIBILITY which is per paramter per shader stage)
struct ROOT_SIGNATURE_FLAGS
{
	static constexpr D3D12_ROOT_SIGNATURE_FLAGS ALLOW_IA_MINIMAL =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

};

struct ROOT_DESCRIPTION
{
	static constexpr D3D12_ROOT_SIGNATURE_DESC1 custom(
		UINT num_parameters,
		const D3D12_ROOT_PARAMETER1* parameters,
		D3D12_ROOT_SIGNATURE_FLAGS flags,
		UINT num_static_samplers = 0,
		const D3D12_STATIC_SAMPLER_DESC* static_samplers = nullptr
		) noexcept
	{
		D3D12_ROOT_SIGNATURE_DESC1 desc{};

		desc.NumParameters = num_parameters;
		desc.pParameters = parameters;
		desc.NumStaticSamplers = num_static_samplers;
		desc.pStaticSamplers = static_samplers;
		desc.Flags = flags;

		return desc;
	}
};

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
struct BARRIERS 
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
struct DESC_HEAP 
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

	static constexpr D3D12_DESCRIPTOR_HEAP_DESC DSV(UINT desc_count) noexcept
	{
		return custom(desc_count, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}
};
