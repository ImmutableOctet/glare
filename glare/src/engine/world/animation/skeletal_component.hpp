#pragma once

#include <engine/types.hpp>

namespace engine
{
	/*
		Simple component indicating that this object has a skeleton attached and that
		`root_bone` is the top-most bone entity in the relationship tree.
		
		`SkeletalComponent` should only be applied to the owning entity,
		there is no need to attach this component to individual bones.
		
		A root bone does not have to be present in order to attach this component.
		(i.e. you can make an entity capable of linking bones without it actually having them yet)

		Entities with this component are referenced by every bone attached via the `BoneComponent::skeleton` field.
	*/
	struct SkeletalComponent
	{
		Entity root_bone;
	};

	/*
		Attaches a `SkeletalComponent` to the `entity` specified.

		If attachment was successful, then `root_bone` will be assigned and this function will return `true`.
		If attachment fails, this routine does nothing, preserving any previously assigned root bone.
		
		NOTE:
		This routine is called automatically on the parent entity when a bone is added. (see `create_bone`)
		Under normal circumstances, there is no need to manually attach a skeleton to an entity.
	*/
	bool attach_skeleton(World& world, Entity entity, Entity root_bone=null);
}