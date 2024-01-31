#include "skeletal_component.hpp"

#include <engine/registry.hpp>

#include <cassert>

namespace engine
{
	bool attach_skeleton(Registry& registry, Entity entity, Entity root_bone)
	{
		assert(entity != null);

		// Check if we've already attached a skeleton to this entity.
		auto* existing = registry.try_get<SkeletalComponent>(entity);

		if (existing)
		{
			if (existing->root_bone == null)
			{
				existing->root_bone = root_bone;

				// Mark skeletal component as patched.
				registry.patch<SkeletalComponent>(entity);

				return true;
			}

			return false;
		}

		registry.emplace<SkeletalComponent>(root_bone);

		return true;
	}
}