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

// Tracks the state of a particular resource.
// 
// There is a base state of a resource and states for subresources that differ from the primary state.
// 
// For example a resource with 3 subresources: 
//		if base state == subresource state; then the state of the subresources in this class IS NOT tracked
//		if the user calls get_state(subresource), where base state and subresource state are equal
//		the base state is returned
class ResourceState
{
public:

	explicit ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON) noexcept
		: mBaseState(state)
	{
	}

	void set_state(UINT subresource, D3D12_RESOURCE_STATES state)
	{
		if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
			mBaseState = state;
			mSubResourceState.clear();
		}
		else
		{
			mSubResourceState[subresource] = state;
		}
	}

	D3D12_RESOURCE_STATES get_state(UINT subresource) const noexcept {
		auto it = mSubResourceState.find(subresource);

		return it != mSubResourceState.end() ? it->second : mBaseState;
	}

	bool is_subresourced() const noexcept {
		return !mSubResourceState.empty();
	}

	const std::map<UINT, D3D12_RESOURCE_STATES>& get_subresource_map() const noexcept {
		return mSubResourceState;
	}

private:

	D3D12_RESOURCE_STATES mBaseState;
	std::map<UINT, D3D12_RESOURCE_STATES> mSubResourceState;
};

// The main manager class for all GPU resources.
// 
// There should only ever be one ultimate point of truth for all resource states (this class could be static / singleton).
// 
// This class however does not own resources, instead manages the states as a fully independent external.
// Any time a resource is created an entry should be added in the class to manage states.
// 
// Importantly, the class acts as a point of convergence between multiple command lists and threads (one command list per thread).
// 
// The command lists may record independent states which should be set to any resources, batch them, them pass them to the main manager.
// 
// for example:
//		have 10 different texture resources
//		have 10 different command lists all wanting to do something different with those textures
//		the 10 command lists first record a state transition for all 10 resources, this may mean 10 different transitions on 10 different threads at the same time
//		then the transitions are batched and synchronized with the main manager
//		then the command list can record the actual desired commands and repeat

using ThreadLocalResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;

class GlobalResourceStateMap 
{
public:
	GlobalResourceStateMap() = default;

	bool try_emplace(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
	{
		if (!resource)
			return false;

		std::lock_guard lock(mLock);

		return mResourceStateMap.try_emplace(resource, state).second;
	}

	bool erase(ID3D12Resource* resource)
	{
		if (!resource)
			return false;
		std::lock_guard lock(mLock);

		return mResourceStateMap.erase(resource) > 0;
	}

private:
	std::unordered_map<ID3D12Resource*, ResourceState> mResourceStateMap;
	std::mutex mLock;
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
	ThreadLocalResourceStateMap mLocalResourceMap;
};