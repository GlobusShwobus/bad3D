#pragma once

#include <d3d12.h>

namespace easy
{
	constexpr D3D12_ROOT_PARAMETER1 root_parameter_descriptor_table(
		D3D12_SHADER_VISIBILITY        visibility,
		UINT                           range_count,
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
		UINT                    shader_register,
		UINT                    count,
		UINT                    register_space = 0U
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
		D3D12_SHADER_VISIBILITY     visibility,
		UINT                        shader_register,
		UINT                        register_space = 0U,
		D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE
	) noexcept
	{
		D3D12_ROOT_PARAMETER1 desc{};
		desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		desc.ShaderVisibility = visibility;
		desc.Descriptor.RegisterSpace = register_space;
		desc.Descriptor.ShaderRegister = shader_register;
		desc.Descriptor.Flags = flags;
		return desc;
	}

	constexpr D3D12_ROOT_PARAMETER1 root_parameter_SRV(
		D3D12_SHADER_VISIBILITY     visibility,
		UINT                        shader_register,
		UINT                        register_space = 0U,
		D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE
	) noexcept
	{
		D3D12_ROOT_PARAMETER1 desc{};
		desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		desc.ShaderVisibility = visibility;
		desc.Descriptor.RegisterSpace = register_space;
		desc.Descriptor.ShaderRegister = shader_register;
		desc.Descriptor.Flags = flags;
		return desc;
	}

	constexpr D3D12_ROOT_PARAMETER1 root_parameter_UAV(
		D3D12_SHADER_VISIBILITY     visibility,
		UINT                        shader_register,
		UINT                        register_space = 0U,
		D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE
	) noexcept
	{
		D3D12_ROOT_PARAMETER1 desc{};
		desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		desc.ShaderVisibility = visibility;
		desc.Descriptor.RegisterSpace = register_space;
		desc.Descriptor.ShaderRegister = shader_register;
		desc.Descriptor.Flags = flags;
		return desc;
	}

	constexpr D3D12_DESCRIPTOR_RANGE1 root_paramater_CBV_range(
		UINT                         num_descriptors,
		UINT                         shader_register,
		UINT                         register_space = 0U,
		UINT                         offset_from_table_start = 0U,
		D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE
	) noexcept
	{
		return D3D12_DESCRIPTOR_RANGE1{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, num_descriptors,  shader_register , register_space , flags , offset_from_table_start };
	}

	constexpr D3D12_DESCRIPTOR_RANGE1 root_paramater_SRV_range(
		UINT                         num_descriptors,
		UINT                         shader_register,
		UINT                         register_space = 0U,
		UINT                         offset_from_table_start = 0U,
		D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE
	) noexcept
	{
		return D3D12_DESCRIPTOR_RANGE1{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, num_descriptors,  shader_register , register_space , flags , offset_from_table_start };
	}

	constexpr D3D12_DESCRIPTOR_RANGE1 root_paramater_UAV_range(
		UINT                         num_descriptors,
		UINT                         shader_register,
		UINT                         register_space = 0U,
		UINT                         offset_from_table_start = 0U,
		D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE
	) noexcept
	{
		return D3D12_DESCRIPTOR_RANGE1{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, num_descriptors,  shader_register , register_space , flags , offset_from_table_start };
	}

	constexpr D3D12_INPUT_ELEMENT_DESC input_element_position(
		UINT                       semantic_index,
		DXGI_FORMAT                format,
		UINT                       input_slot = 0U,
		UINT                       aligned_byte_offset = D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION input_class = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		UINT                       instance_step_rate = 0U
	) noexcept
	{
		return D3D12_INPUT_ELEMENT_DESC{ "POSITION",  semantic_index , format , input_slot , aligned_byte_offset , input_class , instance_step_rate };
	}

	constexpr D3D12_INPUT_ELEMENT_DESC input_element_color(
		UINT                       semantic_index,
		DXGI_FORMAT                format,
		UINT                       input_slot = 0U,
		UINT                       aligned_byte_offset = D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION input_class = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		UINT                       instance_step_rate = 0U
	) noexcept
	{
		return D3D12_INPUT_ELEMENT_DESC{ "COLOR",  semantic_index , format , input_slot , aligned_byte_offset , input_class , instance_step_rate };
	}

	constexpr D3D12_RASTERIZER_DESC rasterizer_solid_backcull(
		INT                                   DepthBias = 0,
		FLOAT                                 DepthBiasClamp = 0.0f,
		FLOAT                                 SlopeScaledDepthBias = 0.0f,
		BOOL                                  DepthClipEnable = TRUE,
		BOOL                                  MultisampleEnable = FALSE,
		BOOL                                  AntialiasedLineEnable = FALSE,
		UINT                                  ForcedSampleCount = 0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	) noexcept
	{
		return D3D12_RASTERIZER_DESC{ D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, FALSE, DepthBias, DepthBiasClamp, SlopeScaledDepthBias, DepthClipEnable, MultisampleEnable, AntialiasedLineEnable,ForcedSampleCount, ConservativeRaster };
	}

	constexpr D3D12_RASTERIZER_DESC rasterizer_wireframe(
		INT                                   DepthBias = 0,
		FLOAT                                 DepthBiasClamp = 0.0f,
		FLOAT                                 SlopeScaledDepthBias = 0.0f,
		BOOL                                  DepthClipEnable = TRUE,
		BOOL                                  MultisampleEnable = FALSE,
		BOOL                                  AntialiasedLineEnable = FALSE,
		UINT                                  ForcedSampleCount = 0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	) noexcept
	{
		return D3D12_RASTERIZER_DESC{ D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE, FALSE, DepthBias, DepthBiasClamp, SlopeScaledDepthBias, DepthClipEnable, MultisampleEnable, AntialiasedLineEnable,ForcedSampleCount, ConservativeRaster };
	}

	constexpr D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_RTV(
		UINT                        num_descs,
		D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		UINT                        node_mask = 0U
	) noexcept
	{
		return D3D12_DESCRIPTOR_HEAP_DESC{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, num_descs, flags, node_mask };
	}

	constexpr D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_DSV(
		UINT                        num_descs,
		D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		UINT                        node_mask = 0U
	) noexcept
	{
		return D3D12_DESCRIPTOR_HEAP_DESC{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, num_descs, flags, node_mask };
	}

	constexpr D3D12_HEAP_PROPERTIES heap_property_default(
		D3D12_CPU_PAGE_PROPERTY cpu_page_property = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL       memPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		UINT                    creationNodeMask = 1U, 
		UINT                    visibleNodeMask = 1U
	) noexcept
	{
		return D3D12_HEAP_PROPERTIES{ D3D12_HEAP_TYPE_DEFAULT, cpu_page_property,memPoolPreference, creationNodeMask,visibleNodeMask };
	}

	constexpr D3D12_HEAP_PROPERTIES heap_property_upload(
		D3D12_CPU_PAGE_PROPERTY cpu_page_property = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL       memPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		UINT                    creationNodeMask = 1U,
		UINT                    visibleNodeMask = 1U
	) noexcept
	{
		return D3D12_HEAP_PROPERTIES{ D3D12_HEAP_TYPE_UPLOAD, cpu_page_property,memPoolPreference, creationNodeMask,visibleNodeMask };
	}

	constexpr D3D12_RESOURCE_DESC resource_desc_buffer(
		UINT64               byte_width,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE
	) noexcept
	{
		return D3D12_RESOURCE_DESC{ D3D12_RESOURCE_DIMENSION_BUFFER, 0ull, byte_width, 1u, 1, 1, DXGI_FORMAT_UNKNOWN, {1,0}, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags };
	}

	constexpr D3D12_RESOURCE_DESC resource_desc_texture2d(
		UINT64               width, 
		UINT                 height,
		DXGI_FORMAT          format,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE
	) noexcept
	{
		return D3D12_RESOURCE_DESC{ D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0ull, width, height, 1, 1, format, {1,0}, D3D12_TEXTURE_LAYOUT_UNKNOWN, flags };
	}

	constexpr D3D12_VERTEX_BUFFER_VIEW resource_view_vertex(
		D3D12_GPU_VIRTUAL_ADDRESS address, 
		UINT                      size_in_bytes,
		UINT                      stride_in_bytes
	) noexcept
	{
		return D3D12_VERTEX_BUFFER_VIEW{address, size_in_bytes, stride_in_bytes};
	}

	constexpr D3D12_INDEX_BUFFER_VIEW resource_view_index(
		D3D12_GPU_VIRTUAL_ADDRESS address, 
		UINT                      size_in_bytes,
		DXGI_FORMAT               format
	) noexcept
	{
		return D3D12_INDEX_BUFFER_VIEW{ address, size_in_bytes, format };
	}

	constexpr D3D12_DEPTH_STENCIL_VIEW_DESC resource_view_textured2d_DSV(
		DXGI_FORMAT     format, 
		UINT            mip_slice, 
		D3D12_DSV_FLAGS flags = D3D12_DSV_FLAG_NONE
	) noexcept
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC  view{};
		view.Format = format;
		view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		view.Flags = flags;
		view.Texture2D.MipSlice = mip_slice;
		return view;
	}

	constexpr D3D12_RESOURCE_BARRIER resource_barrier_transition(
		ID3D12Resource*              resource,
		D3D12_RESOURCE_STATES        before,
		D3D12_RESOURCE_STATES        after,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		UINT                         sub_resource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
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

namespace Pipeline
{
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

	using ROOT_SIGNATURE = PipelineStateSubobject< ID3D12RootSignature*, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE         >;
	using INPUT_LAYOUT = PipelineStateSubobject< D3D12_INPUT_LAYOUT_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT           >;
	using PRIMITIVE_TOPOLOGY = PipelineStateSubobject< D3D12_PRIMITIVE_TOPOLOGY_TYPE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY     >;
	using VERTEX_SHADER = PipelineStateSubobject< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS                     >;
	using PIXEL_SHADER = PipelineStateSubobject< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS                     >;
	using DSV_FORMAT = PipelineStateSubobject< DXGI_FORMAT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT   >;
	using RTV_FORMATS = PipelineStateSubobject< D3D12_RT_FORMAT_ARRAY, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS  >;
	using RASTERIZER = PipelineStateSubobject< D3D12_RASTERIZER_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER             >;
}