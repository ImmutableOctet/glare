#pragma once

#include "entity_state.hpp"
#include "meta/meta_description.hpp"

#include <optional>

namespace engine
{
	struct EntityDescriptor
	{
		using TypeInfo = MetaDescription;

		// Could also be handled as a map-type, but that would
		// be less efficient for what is normally a small number of states.
		using StateCollection = util::small_vector<EntityState, 16>;

		// Statically assigned components.
		MetaDescription components;

		// Dynamic component permutations, applied as state changes.
		StateCollection states;

		const EntityState* get_state(StringHash name) const;
		EntityState* get_state(StringHash name);

		std::optional<EntityStateIndex> get_state_index(StringHash name) const;

		bool set_state(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, std::string_view state_name) const;
		bool set_state_by_id(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, StringHash state_id) const;
		bool set_state_by_index(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, EntityStateIndex state_index) const;
	};
}