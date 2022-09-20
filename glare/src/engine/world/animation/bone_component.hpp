#pragma once

#include <math/types.hpp>
#include <engine/types.hpp>

// TODO: Look into workarounds for including this header directly.
#include <graphics/skeleton.hpp>

#include <string>

namespace graphics
{
	struct Bone;
	struct Skeleton;
}

namespace engine
{
	// Bones are special entities used for animation.
	// `BoneComponent` is currently only valid to be attached to entities on creation.
	// To create a bone, use the `create_bone`, `build_ligaments` or similar.
	struct BoneComponent
	{
		/*
			The skeleton this bone belongs to.
			
			If this bone's parent (via `Relationship` component) is changed, this will automatically
			reflect the new skeleton of that entity. Likewise, if this entity's parent is changed to a
			bone from a different skeleton, this bone will adopt that bone's `skeleton` field.
			
			See `AnimationSystem::on_parent_changed` for details.
		*/
		Entity skeleton;

		BoneID ID;
		std::string name;

		math::Matrix offset;
	};

	std::tuple<bool, Entity> build_ligaments(World& world, Entity parent, const graphics::Skeleton& skeleton, const std::pair<std::string, graphics::Bone>& bone_entry, bool generate_parent=true, bool check_for_existing=true);

	Entity create_bone(World& world, BoneID bone_id, const math::Matrix& local_transform, const math::Matrix& offset, std::string_view bone_name={}, Entity parent=null, EntityType type=EntityType::Bone);
	Entity create_bone(World& world, const graphics::Bone& bone_data, std::string_view bone_name = {}, Entity parent=null, EntityType type=EntityType::Bone);
}