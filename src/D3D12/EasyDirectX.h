#pragma once

#include <d3d12.h>

namespace easy
{
	namespace custom
	{
		constexpr D3D12_ROOT_PARAMETER1 root_parameter_descriptor(
			D3D12_ROOT_PARAMETER_TYPE type,
			D3D12_SHADER_VISIBILITY visibility,
			UINT shader_register,
			UINT register_space,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags
		) noexcept
		{
			D3D12_ROOT_PARAMETER1 desc{};
			desc.ParameterType = type;
			desc.ShaderVisibility = visibility;
			desc.Descriptor.ShaderRegister = shader_register;
			desc.Descriptor.RegisterSpace = register_space;
			desc.Descriptor.Flags = flags;
			return desc;
		}

		constexpr D3D12_DESCRIPTOR_RANGE1 descriptor_range(
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

		constexpr D3D12_INPUT_ELEMENT_DESC input_element_desc(
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

		constexpr D3D12_RASTERIZER_DESC rasterizer_desc(
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

		constexpr D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc(
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			UINT desc_count,
			D3D12_DESCRIPTOR_HEAP_FLAGS flags,
			UINT node_mask
		) noexcept
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc{};
			desc.Type = type;
			desc.NumDescriptors = desc_count;
			desc.NodeMask = node_mask;
			desc.Flags = flags;
			return desc;
		}

		constexpr D3D12_HEAP_PROPERTIES heal_properties(
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

		constexpr D3D12_RESOURCE_DESC resource_desc(
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

		template <typename T, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE TypeValue>
		struct PipelineStateSubobject
		{
			PipelineStateSubobject() = default;

			PipelineStateSubobject(const T& value)
				: object(value)
			{
			}

			PipelineStateSubobject& operator=(const T& value)
			{
				object = value;
				return *this;
			}

			D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type = TypeValue;
			T object{};
		};
	}

	using Pipeline_ROOT_SIGNATURE     = custom::PipelineStateSubobject< ID3D12RootSignature*, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE         >;
	using Pipeline_INPUT_LAYOUT       = custom::PipelineStateSubobject< D3D12_INPUT_LAYOUT_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT           >;
	using Pipeline_PRIMITIVE_TOPOLOGY = custom::PipelineStateSubobject< D3D12_PRIMITIVE_TOPOLOGY_TYPE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY     >;
	using Pipeline_VERTEX_SHADER      = custom::PipelineStateSubobject< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS                     >;
	using Pipeline_PIXEL_SHADER       = custom::PipelineStateSubobject< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS                     >;
	using Pipeline_DSV_FORMAT         = custom::PipelineStateSubobject< DXGI_FORMAT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT   >;
	using Pipeline_RTV_FORMATS        = custom::PipelineStateSubobject< D3D12_RT_FORMAT_ARRAY, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS  >;
	using Pipeline_RASTERIZER         = custom::PipelineStateSubobject< D3D12_RASTERIZER_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER             >;
	
	constexpr D3D12_ROOT_PARAMETER1 root_parameter_descriptor_table(
		D3D12_SHADER_VISIBILITY visibility,
		UINT range_count,
		const D3D12_DESCRIPTOR_RANGE1* range
	) noexcept
	{
		D3D12_ROOT_PARAMETER1 desc{};
		desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.ShaderVisibility = visibility;
		desc.DescriptorTable.NumDescriptorRanges = range_count;
		desc.DescriptorTable.pDescriptorRanges = range;
		return desc;
	}

	constexpr D3D12_ROOT_PARAMETER1 root_parameter_32bit_constants(
		D3D12_SHADER_VISIBILITY visibility,
		UINT shader_register,
		UINT count,
		UINT register_space = 0U
	) noexcept
	{
		D3D12_ROOT_PARAMETER1 desc{};
		desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		desc.ShaderVisibility = visibility;
		desc.Constants.ShaderRegister = shader_register;
		desc.Constants.Num32BitValues = count;
		desc.Constants.RegisterSpace = register_space;
		return desc;
	}

	constexpr D3D12_ROOT_PARAMETER1 root_parameter_CBV(
		D3D12_SHADER_VISIBILITY visibility,
		UINT shader_register,
		UINT register_space = 0U,
		D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE
	) noexcept
	{
		return custom::root_parameter_descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, visibility, shader_register, register_space, flags);
	}

	constexpr D3D12_ROOT_PARAMETER1 root_parameter_SRV(
		D3D12_SHADER_VISIBILITY visibility,
		UINT shader_register,
		UINT register_space = 0U,
		D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE
	) noexcept
	{
		return custom::root_parameter_descriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, visibility, shader_register, register_space, flags);
	}

	constexpr D3D12_ROOT_PARAMETER1 root_parameter_UAV(
		D3D12_SHADER_VISIBILITY visibility,
		UINT shader_register,
		UINT register_space = 0U,
		D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE
	) noexcept
	{
		return custom::root_parameter_descriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, visibility, shader_register, register_space, flags);
	}

	constexpr D3D12_DESCRIPTOR_RANGE1 root_paramater_CBV_range(
		UINT                        count,
		UINT                        shader_register,
		UINT                        register_space = 0U,
		UINT                        offset = 0U,
		D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE
	) noexcept
	{
		return custom::descriptor_range(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, count, shader_register, register_space, offset, flags);
	}

	constexpr D3D12_DESCRIPTOR_RANGE1 root_paramater_SRV_range(
		UINT                        count,
		UINT                        shader_register,
		UINT                        register_space = 0U,
		UINT                        offset = 0U,
		D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE
	) noexcept
	{
		return custom::descriptor_range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, count, shader_register, register_space, offset, flags);
	}

	constexpr D3D12_DESCRIPTOR_RANGE1 root_paramater_UAV_range(
		UINT                        count,
		UINT                        shader_register,
		UINT                        register_space = 0U,
		UINT                        offset = 0U,
		D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE
	) noexcept
	{
		return custom::descriptor_range(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, count, shader_register, register_space, offset, flags);
	}

	constexpr D3D12_INPUT_ELEMENT_DESC input_element_position(
		UINT semantic_index,
		DXGI_FORMAT format,
		UINT input_slot = 0U,
		UINT aligned_byte_offset = D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION input_class = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		UINT instance_step_rate = 0U
	) noexcept
	{
		return custom::input_element_desc("POSITION", semantic_index, format, input_slot, aligned_byte_offset, input_class, instance_step_rate);
	}

	constexpr D3D12_INPUT_ELEMENT_DESC input_element_color(
		UINT semantic_index,
		DXGI_FORMAT format,
		UINT input_slot = 0U,
		UINT aligned_byte_offset = D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION input_class = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		UINT instance_step_rate = 0U
	) noexcept
	{
		return custom::input_element_desc("COLOR", semantic_index, format, input_slot, aligned_byte_offset, input_class, instance_step_rate);
	}

	constexpr D3D12_RASTERIZER_DESC rasterizer_solid_backcull(
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
		return custom::rasterizer_desc(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, FALSE, DepthBias, DepthBiasClamp, SlopeScaledDepthBias, DepthClipEnable, MultisampleEnable, AntialiasedLineEnable, ForcedSampleCount, ConservativeRaster);
	}

	constexpr D3D12_RASTERIZER_DESC rasterizer_wireframe(
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
		return custom::rasterizer_desc(D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE, FALSE, DepthBias, DepthBiasClamp, SlopeScaledDepthBias, DepthClipEnable, MultisampleEnable, AntialiasedLineEnable, ForcedSampleCount, ConservativeRaster);
	}

	constexpr D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_RTV(
		UINT desc_count,
		D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		UINT node_mask = 0U
	) noexcept
	{
		return custom::descriptor_heap_desc(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, desc_count, flags, node_mask);
	}

	constexpr D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_DSV(
		UINT desc_count,
		D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		UINT node_mask = 0U
	) noexcept
	{
		return custom::descriptor_heap_desc(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, desc_count, flags, node_mask);
	}

	constexpr D3D12_HEAP_PROPERTIES heap_property_default(
		D3D12_CPU_PAGE_PROPERTY cpu_page_property = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL memPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		UINT creationNodeMask = 1U, UINT visibleNodeMask = 1U
	) noexcept
	{
		return custom::heal_properties(D3D12_HEAP_TYPE_DEFAULT, cpu_page_property, memPoolPreference, creationNodeMask, visibleNodeMask);
	}

	constexpr D3D12_HEAP_PROPERTIES heap_property_upload(
		D3D12_CPU_PAGE_PROPERTY cpu_page_property = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL memPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		UINT creationNodeMask = 1U,
		UINT visibleNodeMask = 1U
	) noexcept
	{
		return custom::heal_properties(D3D12_HEAP_TYPE_UPLOAD, cpu_page_property, memPoolPreference, creationNodeMask, visibleNodeMask);
	}

	constexpr D3D12_RESOURCE_DESC resource_desc_buffer(UINT64 byte_width, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) noexcept
	{
		return custom::resource_desc(
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

	constexpr D3D12_RESOURCE_DESC resource_desc_texture2d(UINT64 width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) noexcept
	{
		return custom::resource_desc(
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

	constexpr D3D12_VERTEX_BUFFER_VIEW resource_view_vertex(D3D12_GPU_VIRTUAL_ADDRESS address, UINT size_in_bytes, UINT stride_in_bytes) noexcept
	{
		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = address;
		view.SizeInBytes = size_in_bytes;
		view.StrideInBytes = stride_in_bytes;
		return view;
	}

	constexpr D3D12_INDEX_BUFFER_VIEW resource_view_index(D3D12_GPU_VIRTUAL_ADDRESS address, UINT size_in_bytes, DXGI_FORMAT format) noexcept
	{
		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = address;
		view.SizeInBytes = size_in_bytes;
		view.Format = format;
		return view;
	}

	constexpr D3D12_DEPTH_STENCIL_VIEW_DESC resource_view_textured2d_DSV(DXGI_FORMAT format, UINT mip_slice, D3D12_DSV_FLAGS flags = D3D12_DSV_FLAG_NONE) noexcept
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC  view{};
		view.Format = format;
		view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		view.Flags = flags;
		view.Texture2D.MipSlice = mip_slice;
		return view;
	}

	constexpr D3D12_RESOURCE_BARRIER resource_barrier_transition(
		ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before,
		D3D12_RESOURCE_STATES after,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		UINT sub_resource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
	) noexcept
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
}