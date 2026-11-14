#include "D3D12/ResourceStateTracker.h"

#include "D3D12/CommandList.h"
#include "GPUResource.h"

void ResourceStateTracker::resource_barrier(const D3D12_RESOURCE_BARRIER& barrier)
{
	// non transition barriers don't require state tracking
	if (barrier.Type != D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) {
		mResourceBarriers.push_back(barrier);
		return;
	}

	const D3D12_RESOURCE_TRANSITION_BARRIER& transition = barrier.Transition;
	const auto it = mLocalResourceMap.find(transition.pResource);

	// if the resource is already tracked locally, meaning this is a second insertion, then we know the before state, which is the after state from the last time (also cancer)
	if (it != mLocalResourceMap.end()) {
		const ResourceState& state = it->second;

		// if the new transition applies to all sub resources (and there are sub resources...)
		// then a new transition must be pushed for each subresource transition (where after state does not already equal transions after state - slight optimization)
		if (transition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && state.is_subresourced()) {
			for (const auto& subresource : state.get_subresource_map()) {
				if (transition.StateAfter != subresource.second) {
					D3D12_RESOURCE_BARRIER new_barrier = barrier;
					new_barrier.Transition.Subresource = subresource.first;
					new_barrier.Transition.StateBefore = subresource.second;
					mResourceBarriers.push_back(new_barrier);
				}
			}
		}
		else { // the transition applies to a single specific subresource then push only that
			const D3D12_RESOURCE_STATES before_state_for_subresource = state.get_state(transition.Subresource);
			if (transition.StateAfter != before_state_for_subresource) {
				D3D12_RESOURCE_BARRIER new_barrier = barrier;
				new_barrier.Transition.StateBefore = before_state_for_subresource;
				mResourceBarriers.push_back(new_barrier);
			}
		}

	}
	else { // if the resource is not yet tracked locally, then we do not know the before state. push it to the pending barrier to resolve later
		mPendingResourceBarriers.push_back(barrier);
	}

	// whether the resource has been tracked or not before this call, its final state is pushed, possibly replacing the previously known state of the resource
	mLocalResourceMap[transition.pResource].set_state(transition.Subresource, transition.StateAfter);
}

void ResourceStateTracker::transition_barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource)
{
	if (resource) {
		D3D12_RESOURCE_BARRIER transition;
		transition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transition.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transition.Transition.pResource = resource;
		transition.Transition.StateAfter = stateAfter;
		transition.Transition.Subresource = subResource;
		resource_barrier(transition);
	}
}

void ResourceStateTracker::transition_barrier(const GPUResource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource)
{
	transition_barrier(resource.get(), stateAfter,subResource);
}

void ResourceStateTracker::UAV_barrier(const GPUResource* resource)
{
	if (!resource && !resource->get())
		throw std::runtime_error{"nullptr"};
	
	D3D12_RESOURCE_BARRIER uav;
	uav.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uav.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	uav.UAV.pResource = resource->get();
	
	resource_barrier(uav);
}

void ResourceStateTracker::alias_barrier(const GPUResource* resourceBefore, const GPUResource* resourceAfter)
{
	ID3D12Resource* pResourceBefore = resourceBefore != nullptr ? resourceBefore->get() : nullptr;
	ID3D12Resource* pResourceAfter = resourceAfter != nullptr ? resourceAfter->get() : nullptr;

	D3D12_RESOURCE_BARRIER alias;
	alias.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
	alias.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	alias.Aliasing.pResourceAfter = pResourceAfter;
	alias.Aliasing.pResourceBefore = pResourceBefore;

	resource_barrier(alias);
}

void ResourceStateTracker::push_batched_barriers(CommandList& commandList)
{
	UINT num_barreirs = static_cast<UINT>(mResourceBarriers.size());

	if (num_barreirs > 0) {
		auto* d3d12commandlist = commandList.get_graphics_command_list();

		d3d12commandlist->ResourceBarrier(num_barreirs, mResourceBarriers.data());
		mResourceBarriers.clear();
	}
}

uint32_t ResourceStateTracker::flush_pending_barriers(CommandList& list)
{
	assert(mStateMap.is_locked());

	// Resolve the pending resource barriers by checking the global state of the 
    // (sub)resources. Add barriers if the pending state and the global state do
    //  not match.
	std::vector<D3D12_RESOURCE_BARRIER> resource_barriers;
	resource_barriers.reserve(mPendingResourceBarriers.size());

	for (auto& pending_barriers : mPendingResourceBarriers)
	{
		if (pending_barriers.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
		{
			auto& pending_transition = pending_barriers.Transition;

			const auto& iter = mStateMap.find(pending_transition.pResource);

			if (iter != mStateMap.end()) {

				// if all subresources are being transitioned and there are multiple subresources of the resource that are in different state
				auto& resource_state = iter->second;
				if (pending_transition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !resource_state.mSubResourceStates.empty()) {
					// transition all subresources
					for (auto subresourceState : resource_state.mSubResourceStates)
					{
						if (pending_transition.StateAfter != subresourceState.second)
						{
							D3D12_RESOURCE_BARRIER new_barrier = pending_barriers;
							new_barrier.Transition.Subresource = subresourceState.first;
							new_barrier.Transition.StateBefore = subresourceState.second;
							resource_barriers.push_back(new_barrier);
						}
					}
				}

			}
			else {
				// no subresources need to be transitioned. just add as single barrier if needed
				auto globalstate = (iter->second).get_subresource_state(pending_transition.Subresource);
				if (pending_transition.StateAfter != globalstate) {
					//fix up the before state based on current global state of the resource
					pending_barriers.Transition.StateBefore = globalstate;
					resource_barriers.push_back(pending_barriers);
				}

			}

		}
	}

	UINT num_barriers = static_cast<UINT>(resource_barriers.size());

	if (num_barriers > 0) {
		auto* d3d12commandlsit = list.get_graphics_command_list();
		d3d12commandlsit->ResourceBarrier(num_barriers, resource_barriers.data());
	}

	mPendingResourceBarriers.clear();

	return num_barriers;
}

void ResourceStateTracker::commit_final_resource_states()
{
	assert(mStateMap.is_locked());

	// commit final resource states to the global resource state map
	for (const auto& resource_state : mFinalResourceState)
	{
		auto iter = mStateMap.find(resource_state.first);
		iter->second = resource_state.second;
	}

	mFinalResourceState.clear();
}
