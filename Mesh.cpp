#include "Mesh.h"

//	#include <assert.h>
//	#include <utility>
//	#include "EasyDirectXUtils.h"
//	Mesh::Mesh(VertexBuffer vertex_buffer, IndexBuffer index_buffer)
//		:mVertexBuffer(std::move(vertex_buffer)), mIndexBuffer(std::move(index_buffer))
//	{
//	}
//	
//	Microsoft::WRL::ComPtr<ID3D12Resource> Mesh::upload_to_index_buffer(ID3D12Device4* device, ID3D12GraphicsCommandList2* cl, const std::vector<WORD>& buffer)
//	{
//		if (buffer.empty())
//			return nullptr;
//		else
//			return upload_to_buffer(device, cl, mIndexBuffer.get(), buffer.data(), buffer.size(), sizeof(WORD));
//	}
//	
//	Microsoft::WRL::ComPtr<ID3D12Resource> Mesh::upload_to_buffer(ID3D12Device4* device, ID3D12GraphicsCommandList2* cl, ID3D12Resource* dest, const void* data, UINT64 element_count, UINT type_size)
//	{
//		assert(device && "nullptr");
//		assert(cl && "nullptr");
//	
//		const UINT64 byte_size = element_count * type_size;
//	
//		auto intermediary = copy_buffer_to_resource_and_get_intermediary(
//			device,
//			cl,
//			dest,
//			data,
//			byte_size
//		);
//	
//		return intermediary;
//	}
//	