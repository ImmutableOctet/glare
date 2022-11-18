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

		CollisionConfig
		(
			CollisionGroup group=CollisionGroup::None,
			CollisionGroup solid_mask=CollisionGroup::None,
			CollisionGroup interaction_mask=CollisionGroup::None
		);

		CollisionConfig
		(
			CollisionGroup group,
			CollisionGroup solid_mask,
			CollisionGroup interaction_mask,

			bool enabled
		);

		CollisionConfig(EntityType type);

		// Utility overload for `EntityType` input.
		// If `enabled` is true, `type` will be forwarded as expected.
		// If `enabled` is false, `EntityType::Default` will be passed, resulting in no collision group assignments. (i.e. default construction)
		CollisionConfig(EntityType type, bool enabled);

		CollisionConfig(const CollisionConfig&) = default;
		CollisionConfig(CollisionConfig&&) noexcept = default;

		CollisionConfig& operator=(const CollisionConfig&) = default;
		CollisionConfig& operator=(CollisionConfig&&) noexcept = default;

		bool enabled() const;

		// The full mask represents the final bitmask that should be used for filtering at the outermost level.
		// (i.e. what we can engage with in some form)
		inline CollisionGroup get_full_mask() const { return (interaction_mask | solid_mask); }

		inline explicit operator bool() const
		{
			return enabled();
		}
	};
}