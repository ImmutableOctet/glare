#include "lifetime_delta_component.hpp"

#include <algorithm>

namespace engine
{
	bool LifetimeDeltaComponent::add_created(MetaTypeID component_type_id)
	{
		if (created(component_type_id))
		{
			return false;
		}

		// If this component was already marked as destoryed,
		// remove it from that list, then exit.
		// (i.e. creation + destruction = no lifetime change)
		if (remove_destroyed(component_type_id))
		{
			return false;
		}

		created_components.emplace_back(component_type_id);

		return true;
	}

	bool LifetimeDeltaComponent::remove_created(MetaTypeID component_type_id)
	{
		// NOTE: This could be faster if we used `std::find` + `erase`, but that
		// assumes `created_components` is always in a correct state:
		auto removed_elements = std::remove
		(
			created_components.begin(),
			created_components.end(),

			component_type_id
		);

		if (removed_elements == created_components.end())
		{
			return false;
		}

		created_components.erase(removed_elements, created_components.end());

		return true;
	}

	bool LifetimeDeltaComponent::add_destroyed(MetaTypeID component_type_id)
	{
		if (destroyed(component_type_id))
		{
			return false;
		}

		// Ensure that creation entries are removed.
		// 
		// i.e. if destruction is the latest lifetime state,
		// then we don't want to continue storing the creation.
		// 
		// NOTE: In general, attempting to remove a component that
		// does not currently exist is considered a safe operation.
		remove_created(component_type_id);

		destroyed_components.emplace_back(component_type_id);

		return true;
	}

	bool LifetimeDeltaComponent::remove_destroyed(MetaTypeID component_type_id)
	{
		// NOTE: This could be faster if we used `std::find` + `erase`, but that
		// assumes `destroyed_components` is always in a correct state:
		auto removed_elements = std::remove
		(
			destroyed_components.begin(),
			destroyed_components.end(),

			component_type_id
		);

		if (removed_elements == destroyed_components.end())
		{
			return false;
		}

		destroyed_components.erase(removed_elements, destroyed_components.end());

		return true;
	}

	void LifetimeDeltaComponent::clear_created()
	{
		created_components.clear();
	}

	void LifetimeDeltaComponent::clear_destroyed()
	{
		destroyed_components.clear();
	}

	void LifetimeDeltaComponent::clear()
	{
		clear_created();
		clear_destroyed();
	}

	bool LifetimeDeltaComponent::created(MetaTypeID component_type_id) const
	{
		return
		(
			std::find
			(
				created_components.begin(),
				created_components.end(),
			
				component_type_id
			)
			!=
			created_components.end()
		);
	}

	bool LifetimeDeltaComponent::destroyed(MetaTypeID component_type_id) const
	{
		return
		(
			std::find
			(
				destroyed_components.begin(),
				destroyed_components.end(),
			
				component_type_id
			)
			!=
			destroyed_components.end()
		);
	}
}