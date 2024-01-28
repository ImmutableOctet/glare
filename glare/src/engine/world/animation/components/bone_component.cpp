#include "bone_component.hpp"
#include "skeletal_component.hpp"

#include <engine/meta/hash.hpp>

#include <engine/world/animation/bone.hpp>
#include <engine/world/animation/skeleton.hpp>

#include <engine/components/relationship_component.hpp>
#include <engine/components/name_component.hpp>
#include <engine/components/forwarding_component.hpp>

#include <engine/world/world.hpp>

#include <string>

// Debugging related:
#include <engine/world/graphics_entity.hpp>

namespace engine
{
	std::tuple<bool, Entity> build_ligaments
	(
		World& world,
		Entity parent,
		
		const Skeleton& skeleton,
		const Bone& bone,

		bool generate_parent,
		bool check_for_existing
	)
	{
		if (generate_parent)
		{
			if (const auto& parent_id = bone.parent_name)
			{
				if (auto parent_bone = skeleton.get_bone_by_id(parent_id))
				{
					// NOTE: Recursion.
					auto new_parent = std::get<1>
					(
						build_ligaments
						(
							world, parent,
							
							skeleton,
							*parent_bone,

							true, true
						)
					);

					if (new_parent != null)
					{
						parent = new_parent;
					}
				}
			}
		}

		const auto bone_index = skeleton.get_bone_index(&bone);

		assert(bone_index);

		if (check_for_existing)
		{
			auto existing = world.get_bone_by_index(parent, *bone_index, true);

			if (existing != null)
			{
				return { false, existing };
			}
		}

		const auto bone_name = get_known_string_from_hash(bone.name);

		return { true, create_bone(world, *bone_index, bone, bone_name, parent) };
	}

	Entity create_bone
	(
		World& world,
		
		BoneIndex bone_index,
		const Bone& bone,
		
		std::string_view bone_name,
		
		Entity parent,
		EntityType type
	)
	{
		auto& registry = world.get_registry();

		auto entity = engine::create_pivot(world, parent, type);

		// Debugging related:
		//auto entity = engine::load_model(world, "assets/geometry/cone.b3d", parent, type, false);

		if (!bone_name.empty())
		{
			registry.emplace<NameComponent>(entity, std::string { bone_name });
		}

		Entity skeleton = null;

		if (parent != null)
		{
			// Check if the parent entity is also a bone:
			if (auto parent_bone_component = registry.try_get<BoneComponent>(parent))
			{
				// Forward the `skeleton` field from the parent entity.
				// (Points to top-most parent in chain; see below)
				skeleton = parent_bone_component->skeleton;
			}
			else
			{
				// This parent entity isn't a bone, attach a skeleton to it.
				// NOTE: The root bone will be set to `entity` if no skeleton existed previously.
				attach_skeleton(world.get_registry(), parent, entity);

				// Set our skeleton instance to `parent` regardless of root/nested status.
				skeleton = parent;
			}
		}

		registry.emplace<BoneComponent>(entity, skeleton, bone_index);

		// Forward relevant events, logic, etc. to the `skeleton` entity.
		registry.emplace<ForwardingComponent>(entity, skeleton);

		{
			const auto& local_transform = bone.node_transform;

			auto tform = world.get_transform(entity);

			tform.set_local_matrix(local_transform);
		}

		return entity;
	}
}