#pragma once

/*
The ResourceStateTracker class tracks the state of a resource within a command list. 
The class shown here is intended to be used by the custom CommandList class that is described
later in this lesson. It is not intended to be used outside of the CommandList class and
certain assumptions have been made in the design of the ResourceStateTracker class.
For example, while it is possible to build different command lists across different threads,
a single command list will not be shared across multiple threads 
(command list building is a single-threaded operation). 
This allows for some simplifying assumptions such as the state of a resource
will not be modified by multiple threads within the same command list.
*/

#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>

#include <d3d12.h>

// todo: the thread safety is fucked up because of deviation from the guide

class CommandList;
class GPUResource;

class SubresourceState 
{
public:

	SubresourceState(uint32_t subresource_count, D3D12_RESOURCE_STATES initial) 
		:mStates(subresource_count, initial)
	{
	}

	void set_state(UINT subresource_index, D3D12_RESOURCE_STATES state) {

		if (subresource_index == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
			std::fill(mStates.begin(), mStates.end(), state);
		else
			mStates.at(subresource_index) = state;
	}

	D3D12_RESOURCE_STATES get_state(UINT subresource_index) const {
		return mStates.at(subresource_index);
	}

	auto begin() const noexcept {
		return mStates.begin();
	}

	auto end() const noexcept {
		return mStates.end();
	}

	uint32_t subresource_count() const noexcept {
		return (uint32_t)mStates.size();
	}

private:

	std::vector<D3D12_RESOURCE_STATES> mStates;
};

using LocalResourceStateMap = std::unordered_map<ID3D12Resource*, SubresourceState>;
using CentralResourceStateMap = std::unordered_map<ID3D12Resource*, SubresourceState>;

struct LockedCentralResourceStateMap
{
	LockedCentralResourceStateMap(CentralResourceStateMap& map, std::mutex& lock)
		:mCentralStateMap(map), mLock(lock)
	{
	}
	CentralResourceStateMap& mCentralStateMap;
	std::unique_lock<std::mutex> mLock;
};





class ResourceStateTracker
{
public:
	ResourceStateTracker() = default;

	//push a resource barrier to the resource state tracker
	void resource_barrier(const D3D12_RESOURCE_BARRIER& barrier);

	// push a transition resource barrier
	void transition_barrier(
		ID3D12Resource* resource, 
		D3D12_RESOURCE_STATES stateAfter, 
		UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
	);

	void transition_barrier(
		const GPUResource& resource, 
		D3D12_RESOURCE_STATES stateAfter,
		UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
	);

	// push UAV resource barrier for the given resource
	// non-null == for this specific resource
	// null == for any UAV resource
	// but since im an asshole i don't allow null
	void UAV_barrier(const GPUResource* resource);


	// push an aliasing barrier for the given resource
	// beforeResource == The resource currently occupying the space in the heap.
	// afterResource == The resource that will be occupying the space in the heap.
	// 
	// Either the beforeResource or the afterResource parameters can be NULL which
	// indicates that any placed or reserved resource could cause aliasing.
	void alias_barrier(const GPUResource* resourceBefore = nullptr, const GPUResource* resourceAfter = nullptr);


	// flush any pending resource barriers to the command list
	void push_batched_barriers(CommandList& commandList);

	// flush pending shmending
	uint32_t flush_pending_barriers(CommandList& list);

	// commit final resource states to the global resource state map
	// this must be called when the command list is closed
	void commit_final_resource_states();


	// reset state stracking. this is done when the command list is reset
	void reset()
	{
		mPendingResourceBarriers.clear();
		mResourceBarriers.clear();
		mLocalResourceMap.clear();
	}


private:

	// Pending resource transitions are committed before a command list
	// is executed on the command queue. This guarantees that resources will
	// be in the expected state at the beginning of a command list.
	std::vector<D3D12_RESOURCE_BARRIER> mPendingResourceBarriers;

	// Resource barriers that need to be committed to the command list.
	std::vector<D3D12_RESOURCE_BARRIER> mResourceBarriers;

	// The final (last known state) of the resources within a command list.
    // The final resource state is committed to the global resource state when the 
    // command list is closed but before it is executed on the command queue.
	LocalResourceStateMap mLocalResourceMap;
};