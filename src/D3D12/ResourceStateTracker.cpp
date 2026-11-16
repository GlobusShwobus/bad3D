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

		// if the new transition applies to all sub resources
		// then a new transition must be pushed for each subresource transition (where after state does not already equal transions after state - slight optimization)
		if (transition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !state.is_uniform()) {
			for (const auto& subresource : state) {
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

void push_barriers(CommandList& commandList, std::vector<D3D12_RESOURCE_BARRIER> barriers)
{
	UINT num_barreirs = static_cast<UINT>(barriers.size());

	if (num_barreirs > 0) {
		auto* d3d12commandlist = commandList.get_graphics_command_list();

		d3d12commandlist->ResourceBarrier(num_barreirs, barriers.data());
	}
}

std::vector<D3D12_RESOURCE_BARRIER> resolve_pending_barriers(std::vector<D3D12_RESOURCE_BARRIER> pending_barriers, const CentralResourceStateMap& central_map)
{
	std::vector<D3D12_RESOURCE_BARRIER> resource_barriers;
	resource_barriers.reserve(pending_barriers.size());

	for (const D3D12_RESOURCE_BARRIER& barrier : pending_barriers)
	{
		// non transitions do not need to be resolved (though this case should not happen to begin with)
		if (barrier.Type != D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
			continue;

		const D3D12_RESOURCE_TRANSITION_BARRIER& transition = barrier.Transition;
		const auto& it = central_map.find(transition.pResource);

		// resolve if the central map tracks the resource (which is always should...), otherwise not possible and something will break at somepoint
		if (it != central_map.end()) {
			// if all subresources are being transitioned and there are multiple subresources of the resource that are in different state
			const ResourceState& state = it->second;
			if (transition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !state.is_uniform()) {
				// transition all subresources
				for (const auto& subresource : state) {
					if (transition.StateAfter != subresource.second) {
						D3D12_RESOURCE_BARRIER new_barrier = barrier;
						new_barrier.Transition.Subresource = subresource.first;
						new_barrier.Transition.StateBefore = subresource.second;
						resource_barriers.push_back(new_barrier);
					}
				}
			}
			else {
				// no subresources need to be transitioned. just add as single barrier if needed
				D3D12_RESOURCE_STATES state = (it->second).get_state(transition.Subresource);
				if (transition.StateAfter != state) {
					D3D12_RESOURCE_BARRIER new_barrier = barrier;
					new_barrier.Transition.StateBefore = state;
					resource_barriers.push_back(new_barrier);
				}
			}
		}
	}

	return resource_barriers;
}

void sync_central_resource_tracker_with_local(CentralResourceStateMap& central, LocalResourceStateMap local)
{
	// commit final resource states to the global resource state map
	for (const auto& resource_state : local)
	{
		auto it = central.find(resource_state.first);
		it->second = resource_state.second;
	}
}
