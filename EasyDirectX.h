#pragma once

#include <d3d12.h>

struct INPUT_ELEMENT
{	
	static constexpr D3D12_INPUT_ELEMENT_DESC custom(
		LPCSTR SemanticName,
		UINT SemanticIndex,
		DXGI_FORMAT Format,
		UINT InputSlot,
		UINT AlignedByteOffset,
		D3D12_INPUT_CLASSIFICATION InputSlotClass,
		UINT InstanceDataStepRate
	) noexcept
	{
		D3D12_INPUT_ELEMENT_DESC desc = {};
		desc.SemanticName = SemanticName;
		desc.SemanticIndex = SemanticIndex;
		desc.Format = Format;
		desc.InputSlot = InputSlot;
		desc.AlignedByteOffset = AlignedByteOffset;
		desc.InputSlotClass = InputSlotClass;
		desc.InstanceDataStepRate = InstanceDataStepRate;
		return desc;
	}

	static constexpr D3D12_INPUT_ELEMENT_DESC position(UINT semantic_index, DXGI_FORMAT format, UINT input_slot = 0U, UINT aligned_byte_offset = D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION input_class = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, UINT instance_step_rate = 0U) noexcept
	{
		return custom("POSITION", semantic_index, format, input_slot, aligned_byte_offset, input_class, instance_step_rate);
	}

	static constexpr D3D12_INPUT_ELEMENT_DESC color(UINT semantic_index, DXGI_FORMAT format, UINT input_slot = 0U, UINT aligned_byte_offset = D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION input_class = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, UINT instance_step_rate = 0U) noexcept
	{
		return custom("COLOR", semantic_index, format, input_slot, aligned_byte_offset, input_class, instance_step_rate);
	}
};

struct ROOT_PARAMETER
{
	struct DESCRIPTOR_RANGE
	{
		static constexpr D3D12_DESCRIPTOR_RANGE1 custom(
			D3D12_DESCRIPTOR_RANGE_TYPE type,
			UINT                        count,
			UINT                        shader_register,
			UINT                        register_space,
			UINT                        offset,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags
		) noexcept
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

		static constexpr D3D12_DESCRIPTOR_RANGE1 SRV(UINT count, UINT shader_register, UINT register_space = 0U, UINT offset = 0U, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
		{
			return custom( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, count, shader_register, register_space, offset, flags );
		}

		static constexpr D3D12_DESCRIPTOR_RANGE1 CBV(UINT count, UINT shader_register, UINT register_space = 0U, UINT offset = 0U, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
		{
			return custom( D3D12_DESCRIPTOR_RANGE_TYPE_CBV, count, shader_register, register_space, offset, flags );
		}

		static constexpr D3D12_DESCRIPTOR_RANGE1 UAV(UINT count, UINT shader_register, UINT register_space = 0U, UINT offset = 0U, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
		{
			return custom( D3D12_DESCRIPTOR_RANGE_TYPE_UAV, count, shader_register, register_space, offset, flags );
		}

		static constexpr D3D12_DESCRIPTOR_RANGE1 sampler(UINT count, UINT shader_register, UINT register_space = 0U, UINT offset = 0U, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
		{
			return custom( D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, count, shader_register, register_space, offset, flags );
		}
	};

	struct DESCRIPTOR
	{
		static constexpr D3D12_ROOT_DESCRIPTOR1 descriptor(UINT shader_register, UINT register_space = 0U, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
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
		static constexpr D3D12_ROOT_CONSTANTS constant(UINT shader_register, UINT count, UINT register_space = 0U) noexcept
		{
			D3D12_ROOT_CONSTANTS desc{};
			desc.ShaderRegister = shader_register;
			desc.Num32BitValues = count;
			desc.RegisterSpace = register_space;
			return desc;
		}
	};

	static constexpr D3D12_ROOT_PARAMETER1 constant(D3D12_SHADER_VISIBILITY visibility, UINT shader_register, UINT count, UINT register_space = 0U) noexcept
	{
		D3D12_ROOT_PARAMETER1 desc{};
		desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		desc.ShaderVisibility = visibility;
		desc.Constants = CONSTANT::constant(shader_register, count, register_space);
		return desc;
	}

	static constexpr D3D12_ROOT_PARAMETER1 descriptor(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility, UINT shader_register, UINT register_space = 0U, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
	{
		D3D12_ROOT_PARAMETER1 desc{};
		desc.ParameterType = type;
		desc.ShaderVisibility = visibility;
		desc.Descriptor = DESCRIPTOR::descriptor(shader_register, register_space, flags);
		return desc;
	}

	static constexpr D3D12_ROOT_PARAMETER1 descriptor_CBV(D3D12_SHADER_VISIBILITY visibility, UINT shader_register, UINT register_space = 0U, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
	{
		return descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, visibility, shader_register, register_space, flags);
	}

	static constexpr D3D12_ROOT_PARAMETER1 descriptor_SRV(D3D12_SHADER_VISIBILITY visibility, UINT shader_register, UINT register_space = 0U, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
	{
		return descriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, visibility, shader_register, register_space, flags);
	}

	static constexpr D3D12_ROOT_PARAMETER1 descriptor_UAV(D3D12_SHADER_VISIBILITY visibility, UINT shader_register, UINT register_space = 0U, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
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

struct ROOT_SIGNATURE_FLAGS
{
	static constexpr D3D12_ROOT_SIGNATURE_FLAGS ALLOW_IAIL_VS =
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
		UINT num_static_samplers,
		const D3D12_STATIC_SAMPLER_DESC* static_samplers
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

	static constexpr D3D12_ROOT_SIGNATURE_DESC1 description( UINT num_parameters, const D3D12_ROOT_PARAMETER1* parameters, D3D12_ROOT_SIGNATURE_FLAGS flags , UINT num_static_samplers = 0U, const D3D12_STATIC_SAMPLER_DESC* static_samplers = nullptr) noexcept
	{
		return custom(num_parameters, parameters, flags, num_static_samplers, static_samplers);
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

namespace PSS
{
	using ROOT_SIGNATURE     = PIPELINE_STATE_STREAM_OBJECT< ID3D12RootSignature*,          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE         >;
	using INPUT_LAYOUT       = PIPELINE_STATE_STREAM_OBJECT< D3D12_INPUT_LAYOUT_DESC,       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT           >;
	using PRIMITIVE_TOPOLOGY = PIPELINE_STATE_STREAM_OBJECT< D3D12_PRIMITIVE_TOPOLOGY_TYPE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY     >;
	using VERTEX_SHADER      = PIPELINE_STATE_STREAM_OBJECT< D3D12_SHADER_BYTECODE,         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS                     >;
	using PIXEL_SHADER       = PIPELINE_STATE_STREAM_OBJECT< D3D12_SHADER_BYTECODE,         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS                     >;
	using DSV_FORMAT         = PIPELINE_STATE_STREAM_OBJECT< DXGI_FORMAT,                   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT   >;
	using RTV_FORMATS        = PIPELINE_STATE_STREAM_OBJECT< D3D12_RT_FORMAT_ARRAY,         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS  >;
	using RASTERIZER         = PIPELINE_STATE_STREAM_OBJECT< D3D12_RASTERIZER_DESC,         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER             >;
}

// heap type shit (currently have not used HEAP DESC regular)
struct DESCRIPTOR_HEAP_DESC 
{
	static constexpr D3D12_DESCRIPTOR_HEAP_DESC custom(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT desc_count, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT node_mask) noexcept
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Type = type;
		desc.NumDescriptors = desc_count;
		desc.NodeMask = node_mask;
		desc.Flags = flags;
		return desc;
	}

	static constexpr D3D12_DESCRIPTOR_HEAP_DESC RTV(UINT desc_count, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, UINT node_mask = 0U) noexcept
	{
		return custom(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, desc_count, flags, node_mask);
	}

	static constexpr D3D12_DESCRIPTOR_HEAP_DESC DSV(UINT desc_count, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, UINT node_mask = 0U) noexcept
	{
		return custom(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, desc_count, flags, node_mask);
	}
};

struct HEAP_PROPERTY
{
	static constexpr D3D12_HEAP_PROPERTIES custom(
		D3D12_HEAP_TYPE type,
		D3D12_CPU_PAGE_PROPERTY cpu_page_property,
		D3D12_MEMORY_POOL memPoolPreference,
		UINT creationNodeMask,
		UINT visibleNodeMask
	) noexcept
	{
		D3D12_HEAP_PROPERTIES desc{};
		desc.Type = type;
		desc.CPUPageProperty = cpu_page_property;
		desc.MemoryPoolPreference = memPoolPreference;
		desc.CreationNodeMask = creationNodeMask;
		desc.VisibleNodeMask = visibleNodeMask;
		return desc;
	}

	static constexpr D3D12_HEAP_PROPERTIES base( D3D12_CPU_PAGE_PROPERTY cpu_page_property = D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL memPoolPreference = D3D12_MEMORY_POOL_UNKNOWN, UINT creationNodeMask = 1U, UINT visibleNodeMask = 1U ) noexcept
	{
		return custom(D3D12_HEAP_TYPE_DEFAULT, cpu_page_property, memPoolPreference, creationNodeMask, visibleNodeMask);
	}

	static constexpr D3D12_HEAP_PROPERTIES upload( D3D12_CPU_PAGE_PROPERTY cpu_page_property = D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL memPoolPreference = D3D12_MEMORY_POOL_UNKNOWN, UINT creationNodeMask = 1U, UINT visibleNodeMask = 1U ) noexcept
	{
		return custom(D3D12_HEAP_TYPE_UPLOAD, cpu_page_property, memPoolPreference, creationNodeMask, visibleNodeMask);
	}
};

// RESOURCE SHIT

struct RESOURCE_DESC
{
	static constexpr D3D12_RESOURCE_DESC custom(
		D3D12_RESOURCE_DIMENSION Dimension,
		UINT64                   Alignment,
		UINT64                   Width,
		UINT                     Height,
		UINT16                   DepthOrArraySize,
		UINT16                   MipLevels,
		DXGI_FORMAT              Format,
		DXGI_SAMPLE_DESC         SampleDesc,
		D3D12_TEXTURE_LAYOUT     Layout,
		D3D12_RESOURCE_FLAGS     Flags
	) noexcept
	{
		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = Dimension;
		desc.Alignment = Alignment;
		desc.Width = Width;
		desc.Height = Height;
		desc.DepthOrArraySize = DepthOrArraySize;
		desc.MipLevels = MipLevels;
		desc.Format = Format;
		desc.SampleDesc = SampleDesc;
		desc.Layout = Layout;
		desc.Flags = Flags;
		return desc;
	}

	static constexpr D3D12_RESOURCE_DESC buffer(UINT64 byte_width, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) noexcept
	{
		return custom(
			D3D12_RESOURCE_DIMENSION_BUFFER,
			0ULL, 
			byte_width, 
			1U,
			1, 
			1,
			DXGI_FORMAT_UNKNOWN, 
			{ 1, 0 }, 
			D3D12_TEXTURE_LAYOUT_ROW_MAJOR, 
			flags
		);
	}

	static constexpr D3D12_RESOURCE_DESC texture2d(UINT64 width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) noexcept
	{
		return custom(
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			width,
			height,
			1,
			1,
			format,
			{ 1, 0 },
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			flags
		);
	}
};

struct RESOURCE_VIEW
{
	static constexpr D3D12_VERTEX_BUFFER_VIEW vertex(D3D12_GPU_VIRTUAL_ADDRESS address, UINT size_in_bytes, UINT stride_in_bytes) noexcept
	{
		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = address;
		view.SizeInBytes = size_in_bytes;
		view.StrideInBytes = stride_in_bytes;
		return view;
	}

	static constexpr D3D12_INDEX_BUFFER_VIEW index(D3D12_GPU_VIRTUAL_ADDRESS address, UINT size_in_bytes, DXGI_FORMAT format) noexcept
	{
		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = address;
		view.SizeInBytes = size_in_bytes;
		view.Format = format;
		return view;
	}

	static constexpr D3D12_DEPTH_STENCIL_VIEW_DESC textured2d_DSV(DXGI_FORMAT format, UINT mip_slice = 0U, D3D12_DSV_FLAGS flags = D3D12_DSV_FLAG_NONE) noexcept
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC  view{};
		view.Format = format;
		view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		view.Flags = flags;
		view.Texture2D.MipSlice = mip_slice;
		return view;
	}
};

struct RESOURCE_BARRIER
{
	static constexpr D3D12_RESOURCE_BARRIER transition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, UINT sub_resource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) noexcept
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = flags;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = before;
		barrier.Transition.StateAfter = after;
		barrier.Transition.Subresource = sub_resource;
		return barrier;
	}
};

struct RASTERIZER_DESC
{
	static constexpr D3D12_RASTERIZER_DESC custom(
		D3D12_FILL_MODE                       FillMode,
		D3D12_CULL_MODE                       CullMode,
		BOOL                                  FrontCounterClockwise,
		INT                                   DepthBias,
		FLOAT                                 DepthBiasClamp,
		FLOAT                                 SlopeScaledDepthBias,
		BOOL                                  DepthClipEnable,
		BOOL                                  MultisampleEnable,
		BOOL                                  AntialiasedLineEnable,
		UINT                                  ForcedSampleCount,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster
	) noexcept
	{
		D3D12_RASTERIZER_DESC desc{};
		desc.FillMode = FillMode;
		desc.CullMode = CullMode;
		desc.FrontCounterClockwise = FrontCounterClockwise;
		desc.DepthBias = DepthBias;
		desc.DepthBiasClamp = DepthBiasClamp;
		desc.SlopeScaledDepthBias = SlopeScaledDepthBias;
		desc.DepthClipEnable = DepthClipEnable;
		desc.MultisampleEnable = MultisampleEnable;
		desc.AntialiasedLineEnable = AntialiasedLineEnable;
		desc.ForcedSampleCount = ForcedSampleCount;
		desc.ConservativeRaster = ConservativeRaster;
		return desc;
	}

	static constexpr D3D12_RASTERIZER_DESC solid_backcull(
		INT DepthBias = 0,
		FLOAT DepthBiasClamp = 0.0f, 
		FLOAT SlopeScaledDepthBias = 0.0f,
		BOOL DepthClipEnable = TRUE,
		BOOL MultisampleEnable = FALSE,
		BOOL AntialiasedLineEnable = FALSE,
		UINT ForcedSampleCount = 0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		) // from msdn the FillMode, CullMode and FrontCounterClockwise are already the correct defaults so this method is LARP af
	{
		return custom(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, FALSE, DepthBias, DepthBiasClamp, SlopeScaledDepthBias, DepthClipEnable, MultisampleEnable, AntialiasedLineEnable, ForcedSampleCount, ConservativeRaster);
	}

	static constexpr D3D12_RASTERIZER_DESC wireframe(
		INT DepthBias = 0,
		FLOAT DepthBiasClamp = 0.0f,
		FLOAT SlopeScaledDepthBias = 0.0f,
		BOOL DepthClipEnable = TRUE,
		BOOL MultisampleEnable = FALSE,
		BOOL AntialiasedLineEnable = FALSE,
		UINT ForcedSampleCount = 0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	)
	{
		return custom(D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE, FALSE, DepthBias, DepthBiasClamp, SlopeScaledDepthBias, DepthClipEnable, MultisampleEnable, AntialiasedLineEnable, ForcedSampleCount, ConservativeRaster);
	}
};