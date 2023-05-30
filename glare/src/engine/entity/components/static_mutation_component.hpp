#pragma once

#include <engine/types.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/meta_variable_scope.hpp>

#include <engine/entity/static_variable_mutation.hpp>

#include <util/small_vector.hpp>

#include <string_view>

namespace engine
{
	// Describes which elements of an entity have been altered during static instantiation.
	// (e.g. during the scene creation/loading phase.)
	struct StaticMutationComponent
	{
		// A list of component type IDs that have been
		// overridden from the base entity description.
		util::small_vector<MetaTypeID, 8> mutated_components;

		// A list of variable names that have been modified/assigned.
		util::small_vector<StaticVariableMutation, 8> mutated_variables;

		bool add_component(MetaTypeID type_id);
		bool add_component(const MetaType& type);

		bool remove_component(MetaTypeID type_id);
		bool remove_component(const MetaType& type);

		bool contains_component(MetaTypeID type_id) const;
		bool contains_component(const MetaType& type) const;

		bool add_variable(StaticVariableMutation&& variable_assignment);
		bool add_variable(const StaticVariableMutation& variable_assignment);
		
		bool remove_variable(const StaticVariableMutation& variable_assignment);
		bool remove_variable(std::string_view variable_name, MetaVariableScope scope);
		bool remove_variable(std::string_view variable_name);

		bool contains_variable(std::string_view variable_name, MetaVariableScope scope) const;
		bool contains_variable(std::string_view variable_name) const;
	};
}