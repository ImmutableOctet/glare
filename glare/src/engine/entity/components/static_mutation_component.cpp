#include "static_mutation_component.hpp"

#include <engine/meta/meta_type.hpp>

#include <algorithm>

namespace engine
{
	bool StaticMutationComponent::add_component(MetaTypeID type_id)
	{
		if (!type_id)
		{
			return false;
		}

		for (const auto& existing : mutated_components)
		{
			if (existing == type_id)
			{
				return false;
			}
		}

		mutated_components.emplace_back(type_id);

		return true;
	}

	bool StaticMutationComponent::add_component(const MetaType& type)
	{
		if (!type)
		{
			return false;
		}

		return add_component(type.id());
	}

	bool StaticMutationComponent::remove_component(MetaTypeID type_id)
	{
		if (!type_id)
		{
			return false; // true;
		}

		auto component_removal_it = std::remove(mutated_components.begin(), mutated_components.end(), type_id);

		if (component_removal_it != mutated_components.end())
		{
			mutated_components.erase(component_removal_it);

			return true;
		}

		return false;
	}

	bool StaticMutationComponent::remove_component(const MetaType& type)
	{
		if (!type)
		{
			return false; // true;
		}

		return remove_component(type.id());
	}

	bool StaticMutationComponent::contains_component(MetaTypeID type_id) const
	{
		if (!type_id)
		{
			return false;
		}

		auto component_entry_it = std::find(mutated_components.begin(), mutated_components.end(), type_id);

		return (component_entry_it != mutated_components.end());
	}
	
	bool StaticMutationComponent::contains_component(const MetaType& type) const
	{
		if (!type)
		{
			return false;
		}

		return contains_component(type.id());
	}

	bool StaticMutationComponent::add_variable(StaticVariableMutation&& variable_assignment)
	{
		if (variable_assignment.variable_name.empty()) // get_name().empty()
		{
			return false;
		}

		for (const auto& variable_entry : mutated_variables)
		{
			if (variable_entry == variable_assignment)
			{
				return false;
			}
		}

		mutated_variables.emplace_back(std::move(variable_assignment));

		return true;
	}

	bool StaticMutationComponent::add_variable(const StaticVariableMutation& variable_assignment)
	{
		return add_variable(StaticVariableMutation { variable_assignment });
	}

	bool StaticMutationComponent::remove_variable(const StaticVariableMutation& variable_assignment)
	{
		return remove_variable(variable_assignment.get_name(), variable_assignment.get_scope());
	}

	bool StaticMutationComponent::remove_variable(std::string_view variable_name, MetaVariableScope scope)
	{
		if (variable_name.empty())
		{
			return false; // true;
		}

		auto variable_removal_it = std::remove_if
		(
			mutated_variables.begin(), mutated_variables.end(),

			[&variable_name, scope](const StaticVariableMutation& variable_entry)
			{
				return
				(
					(variable_entry.get_scope() == scope)
					&&
					(variable_entry.get_name() == variable_name)
				);
			}
		);

		if (variable_removal_it != mutated_variables.end())
		{
			mutated_variables.erase(variable_removal_it, mutated_variables.end());

			return true;
		}

		return false;
	}

	bool StaticMutationComponent::remove_variable(std::string_view variable_name)
	{
		if (variable_name.empty())
		{
			return false; // true;
		}

		auto variable_removal_it = std::remove_if
		(
			mutated_variables.begin(), mutated_variables.end(),

			[&variable_name](const StaticVariableMutation& variable_entry)
			{
				return (variable_entry.get_name() == variable_name);
			}
		);

		if (variable_removal_it != mutated_variables.end())
		{
			mutated_variables.erase(variable_removal_it, mutated_variables.end());

			return true;
		}

		return false;
	}

	bool StaticMutationComponent::contains_variable(std::string_view variable_name, MetaVariableScope scope) const
	{
		if (variable_name.empty())
		{
			return false;
		}

		auto variable_entry_it = std::find_if
		(
			mutated_variables.begin(), mutated_variables.end(),

			[&variable_name, scope](const StaticVariableMutation& variable_entry)
			{
				return
				(
					(variable_entry.get_scope() == scope)
					&&
					(variable_entry.get_name() == variable_name)
				);
			}
		);

		return (variable_entry_it != mutated_variables.end());
	}

	bool StaticMutationComponent::contains_variable(std::string_view variable_name) const
	{
		if (variable_name.empty())
		{
			return false;
		}

		auto variable_entry_it = std::find_if
		(
			mutated_variables.begin(), mutated_variables.end(),

			[&variable_name](const StaticVariableMutation& variable_entry)
			{
				return (variable_entry.get_name() == variable_name);
			}
		);

		return (variable_entry_it != mutated_variables.end());
	}
}