#include "D3D12/ResourceStateTracker.h"

#include "D3D12/CommandList.h"
#include "GPUResource.h"

void ResourceStateTracker::resource_barrier(const D3D12_RESOURCE_BARRIER& barrier)
{
	if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
	{
		const D3D12_RESOURCE_TRANSITION_BARRIER& transition_barrier = barrier.Transition;

		const auto iter = mFinalResourceState.find(transition_barrier.pResource);

		// if the resource is already tracked in the list, then the before state may immediately be set for the queued barrier
		if (iter != mFinalResourceState.end()) {
			auto& resource_state = iter->second;

			if (transition_barrier.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !resource_state.mSubResourceStates.empty())
			{
				for (auto& subresource_state : resource_state.mSubResourceStates) {

					if (transition_barrier.StateAfter != subresource_state.second) {
						D3D12_RESOURCE_BARRIER new_barrier = barrier;
						new_barrier.Transition.Subresource = subresource_state.first;
						new_barrier.Transition.StateBefore = subresource_state.second;
						mResourceBarriers.push_back(new_barrier);
					}

				}
			}
			else {
				auto final_state = resource_state.get_subresource_state(transition_barrier.Subresource);

				if (transition_barrier.StateAfter != final_state) {
					D3D12_RESOURCE_BARRIER new_barrier = barrier;
					new_barrier.Transition.StateBefore = final_state;
					mResourceBarriers.push_back(new_barrier);
				}
			}
		}
		else { // In this case, the resource is being used on the command list for the first time.
			mPendingResourceBarriers.push_back(barrier);
			mFinalResourceState[transition_barrier.pResource].set_subresource_state(transition_barrier.Subresource, transition_barrier.StateAfter);
		}
	}
	else {
		// non transition barriers don't require state tracking
		mResourceBarriers.push_back(barrier);
	}
}

void ResourceStateTracker::transition_barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource)
{
	if (resource) {
		D3D12_RESOURCE_BARRIER transition;
		transition.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
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
