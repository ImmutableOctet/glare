#pragma once

#include "types.hpp"

#include <math/types.hpp>

namespace engine
{
	// TODO: Review pros/cons of ID-based bone references.
	// TODO: Determine if `Bone` objects should store their name/ID vs. storing this information in the `Skeleton`.
	struct Bone
	{
		// An identifier representing the name of this bone.
		BoneID name;

		// An identifier representing the name of this bone's parent.
		BoneID parent_name;

		// Local matrix transform for the node containing this bone.
		math::Matrix node_transform;

		// Model-space to bone-local-space offset matrix.
		math::Matrix offset;
	};
}