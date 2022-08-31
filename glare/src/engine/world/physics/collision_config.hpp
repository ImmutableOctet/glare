#pragma once

#include <engine/types.hpp> // Included for `EntityType`.

#include "collision_group.hpp"

namespace engine
{
	struct CollisionConfig
	{
		// What group this object belongs to.
		CollisionGroup group = CollisionGroup::None;

		// What groups are solid to this object.
		CollisionGroup solid_mask = CollisionGroup::None;

		// What groups produce an interaction event.
		CollisionGroup interaction_mask = CollisionGroup::None;

		static CollisionGroup resolve_collision_group(EntityType type);
		static CollisionGroup resolve_solid_mask(EntityType type);
		static CollisionGroup resolve_interaction_mask(EntityType type);

		//using CollisionLookupResult = std::tuple<CollisionGroup, CollisionGroup, CollisionGroup>;

		CollisionConfig() = default;
		CollisionConfig(const CollisionConfig&) = default;
		CollisionConfig(CollisionConfig&&) = default;

		CollisionConfig& operator=(CollisionConfig&&) = default;

		CollisionConfig(EntityType type);

		// The full mask represents the final bitmask that should be used for filtering at the outermost level.
		// (i.e. what we can engage with in some form)
		inline CollisionGroup get_full_mask() const { return (interaction_mask | solid_mask); }
	};
}