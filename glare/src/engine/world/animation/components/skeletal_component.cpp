#include "skeletal_component.hpp"

#include "bone_component.hpp"

#include <engine/world/world.hpp>

namespace engine
{
	bool attach_skeleton(World& world, Entity entity, Entity root_bone)
	{
		assert(entity != null);

		auto& registry = world.get_registry();

		// Check if we've already attached a skeleton to this entity.
		auto* existing = registry.try_get<SkeletalComponent>(entity);

		if (existing)
		{
			if (existing->root_bone == null)
			{
				existing->root_bone = root_bone;

				return true;
			}

			return false;
		}

		registry.emplace<SkeletalComponent>(root_bone);

		return true;
	}
}