#pragma once

#include <engine/types.hpp>

#include <math/types.hpp>

#include <string_view>

namespace engine
{
	struct Bone;
	struct Skeleton;

	// Bones are special entities used for animation.
	// `BoneComponent` is currently only valid to be attached to entities on creation.
	// To create a bone, use `create_bone`, `build_ligaments` or similar.
	struct BoneComponent
	{
		/*
			The root entity, owner of the skeleton this bone belongs to.
			
			If this bone's parent (via `RelationshipComponent`) is changed, this will automatically
			reflect the new skeleton of that entity. Likewise, if this entity's parent is changed to a
			bone from a different skeleton, this bone will adopt that bone's `skeleton` field.
			
			See `AnimationSystem::on_parent_changed` for details.
		*/
		Entity skeleton;

		// The index of the bone that this entity represents.
		BoneIndex bone_index;
	};

	std::tuple<bool, Entity> build_ligaments
	(
		World& world,
		Entity parent,
		
		const Skeleton& skeleton,
		const Bone& bone,

		bool generate_parent=true,
		bool check_for_existing=true
	);

	Entity create_bone
	(
		World& world,

		BoneIndex bone_index,
		const Bone& bone,

		std::string_view bone_name={},
		
		Entity parent=null,
		EntityType type=EntityType::Bone
	);
}