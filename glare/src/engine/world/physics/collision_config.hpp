#pragma once

#include <engine/types.hpp> // Included for `EntityType`.

#include "collision_group.hpp"

namespace engine
{
	struct CollisionConfig
	{
		CollisionGroup group = CollisionGroup::None;
		CollisionGroup solid_mask = CollisionGroup::None;
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
	};
}