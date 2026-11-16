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

// This class represents the state of the resource, in two different ways:
//		- there is a uniform state, a state that represents all (if any subresources) states
//		- subresources are not tracked
//		- the different states from the uniform state are tracked
// 
// The class does not know nor care how many potential subresources the resource actually has. It only cares about
// states that are different from the uniform state. That is why set_state erases all entries in the map if D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES is used
// and why get_state returns the uniform state if an unknown subresource is asked for.
class ResourceState 
{
public:

	ResourceState(D3D12_RESOURCE_STATES initial = D3D12_RESOURCE_STATE_COMMON)
		:mUniformState(initial)
	{ }

	void set_state(UINT subresource_index, D3D12_RESOURCE_STATES state) {
		if (subresource_index == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
			mUniformState = state;
			mDifferenceMap.clear();
		}
		else {
			mDifferenceMap[subresource_index] = state;
		}
	}

	D3D12_RESOURCE_STATES get_state(UINT subresource_index) const {
		D3D12_RESOURCE_STATES state = mUniformState;
		const auto iter = mDifferenceMap.find(subresource_index);
		if (iter != mDifferenceMap.end())
			state = iter->second;

		return state;
	}

	bool is_uniform() const noexcept {
		return mDifferenceMap.empty();
	}

	auto begin() const noexcept {
		return mDifferenceMap.begin();
	}

	auto end() const noexcept {
		return mDifferenceMap.end();
	}

private:
	D3D12_RESOURCE_STATES mUniformState;
	std::map<uint32_t, D3D12_RESOURCE_STATES> mDifferenceMap;
};

using LocalResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;
using CentralResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;

class LockedCentralResourceStateMap
{
public:
	LockedCentralResourceStateMap(CentralResourceStateMap& map, std::mutex& lock)
		:mCentralStateMap(map), mLock(lock)
	{
	}

	auto find(ID3D12Resource* p)const {
		return mCentralStateMap.find(p);
	}

	auto end() const noexcept {
		return mCentralStateMap.end();
	}

	CentralResourceStateMap& mCentralStateMap;

private:
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


	std::vector<D3D12_RESOURCE_BARRIER> pass_pending_barriers() noexcept {
		return std::exchange(mPendingResourceBarriers, {});
	}

	std::vector<D3D12_RESOURCE_BARRIER> pass_resolved_barriers() noexcept {
		return std::exchange(mResourceBarriers, {});
	}

	LocalResourceStateMap pass_final_states() noexcept {
		return std::exchange(mLocalResourceMap, {});
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


// push an array of resource barriers to a command list
// this function may be used multiple times in a frame by a single command list
//
// Intended use case: batch up transition barriers for an array of resources then push them all at once. After that other commands may be recorded.
void push_barriers(CommandList& commandList, std::vector<D3D12_RESOURCE_BARRIER> barriers);

// resolve pending barriers by looking up central map and then attaching the before states
//
// intended use case: resolve any pending barriers before passing the relevant command list to the command queue. 
// new commands can't be in front of the command list so a new command list must be used to resolve the issue
// (currently it seems as the resolve pending barriers must be done with every command list)
std::vector<D3D12_RESOURCE_BARRIER> resolve_pending_barriers(std::vector<D3D12_RESOURCE_BARRIER> pending_barriers, const CentralResourceStateMap& central_map);


// sync up command list local state map to the central state map
//
// intended use case: after resolving pending barriers and the relevant command list is closed
void sync_central_resource_tracker_with_local(CentralResourceStateMap& central, LocalResourceStateMap local);