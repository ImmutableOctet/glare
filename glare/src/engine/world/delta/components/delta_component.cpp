#include "delta_component.hpp"

#include <algorithm>

namespace engine
{
	bool DeltaComponent::add_modified(MetaTypeID component_type_id)
	{
		// Check if `component_type_id` has already been marked as modified.
		if (modified(component_type_id))
		{
			return false;
		}

		modified_components.emplace_back(component_type_id);

		return true;
	}
	
	bool DeltaComponent::remove_modified(MetaTypeID component_type_id)
	{
		// NOTE: This could be faster if we used `std::find` + `erase`, but that
		// assumes `modified_components` is always in a correct state:
		auto removed_elements = std::remove
		(
			modified_components.begin(),
			modified_components.end(),

			component_type_id
		);

		if (removed_elements == modified_components.end())
		{
			return false;
		}

		modified_components.erase(removed_elements, modified_components.end());

		return true;
	}
	
	void DeltaComponent::clear_modified()
	{
		modified_components.clear();
	}

	void DeltaComponent::clear()
	{
		clear_modified();
	}

	bool DeltaComponent::modified(MetaTypeID component_type_id) const
	{
		return
		(
			std::find
			(
				modified_components.begin(),
				modified_components.end(),
			
				component_type_id
			)
			!=
			modified_components.end()
		);
	}
}