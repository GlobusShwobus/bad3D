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

// todo make a resource state map instead of static global... then build trackers i guess. no statics tho plz GOD

class CommandList;
class GPUResource;

// Tracks the state of a particular resource and all of its subresources.
struct ResourceState
{
	// Initialize all of the subresources within a resource to the given state.
	explicit ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON)
		: mState(state)
	{
	}

	// Set a subresource to a particular state.
	void set_subresource_state(UINT subresource, D3D12_RESOURCE_STATES state)
	{
		if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
			mState = state;
			mSubResourceStates.clear();
		}
		else
		{
			mSubResourceStates[subresource] = state;
		}
	}

	// Get the state of a (sub)resource within the resource.
    // If the specified subresource is not found in the SubresourceState array (map)
    // then the state of the resource (D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) is
    // returned.
	D3D12_RESOURCE_STATES get_subresource_state(UINT subresource) const
	{
		D3D12_RESOURCE_STATES state = mState;
		const auto iter = mSubResourceStates.find(subresource);
		if (iter != mSubResourceStates.end())
		{
			state = iter->second;
		}
		return state;
	}

	// If the SubresourceState array (map) is empty, then the State variable defines 
	// the state of all of the subresources.
	D3D12_RESOURCE_STATES mState;
	std::map<UINT, D3D12_RESOURCE_STATES> mSubResourceStates;
};


class ResourceStateMap
{
public:
	ResourceStateMap() :mResourceStateMap{}, mLock{}, mIsLocked(false) {}

	void lock();

	void unlock();

	void add_resource(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);

	void remove_resource(ID3D12Resource* resource);

private:
	std::unordered_map<ID3D12Resource*, ResourceState> mResourceStateMap;

	// The mutex protects shared access to the GlobalResourceState map.
	std::mutex mLock;
	bool mIsLocked;
};


class ResourceStateTracker
{
public:
	ResourceStateTracker(ResourceStateMap& state_map)
		:mStateMap(state_map)
	{
	}

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
	void UAV_barrier(const GPUResource* resource = nullptr);


	// push an aliasing barrier for the given resource
	// beforeResource == The resource currently occupying the space in the heap.
	// afterResource == The resource that will be occupying the space in the heap.
	// 
	// Either the beforeResource or the afterResource parameters can be NULL which
	// indicates that any placed or reserved resource could cause aliasing.
	void alias_barrier(const GPUResource* resourceBefore = nullptr, const GPUResource* resourceAfter = nullptr);



	// flush any pending resource barriers to the command list
	uint32_t flush_pending_barriers(CommandList& commandList);


	// commit final resource states to the global resource state map
	// this must be called when the command list is closed
	void commit_final_resource_states();


	// reset state stracking. this is done when the command list is reset
	void reset();

private:

	ResourceStateMap& mStateMap;

	// An array of resource barriers.
	using ResourceBarriers = std::vector<D3D12_RESOURCE_BARRIER>;

	// Pending resource transitions are committed before a command list
	// is executed on the command queue. This guarantees that resources will
	// be in the expected state at the beginning of a command list.
	ResourceBarriers mPendingResourceBarriers;

	// Resource barriers that need to be committed to the command list.
	ResourceBarriers mResourceBarriers;

	// The final (last known state) of the resources within a command list.
    // The final resource state is committed to the global resource state when the 
    // command list is closed but before it is executed on the command queue.
	std::unordered_map<ID3D12Resource*, ResourceState> mFinalResourceState;
};