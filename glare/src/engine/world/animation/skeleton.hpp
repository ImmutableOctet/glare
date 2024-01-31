#pragma once

#include "types.hpp"
#include "bone.hpp"

#include <math/types.hpp>

#include <string_view>
#include <optional>
#include <vector>

//#include <util/small_vector.hpp>

namespace engine
{
	// A primitive type used to store a static description of a skeleton.
	// (Currently handled as a flat collection of `Bone`s; see `Container`)
	// 
	// NOTE: Parent/child relationships are handled by the `Bone` objects themselves.
	struct Skeleton
	{
		// We use a vector-type here instead of a hash-map (or similar) due to the relatively small number of elements per-skeleton on average.
		// TODO: Review pros/cons of switching to map container.
		using BoneContainer = std::vector<Bone>; // util::small_vector<...>

		// Attempts to retrieve a pointer to a `Bone` located at `bone_index`.
		const Bone* get_bone_by_index(BoneIndex bone_index) const;

		// Searches for a bone matching the `bone_id` specified.
		const Bone* get_bone_by_id(BoneID bone_id) const;

		// Searches for a bone whose identifier matches the hash of the `bone_name` specified.
		const Bone* get_bone_by_name(std::string_view bone_name) const;

		// Equivalent to `get_bone_by_name`.
		inline const Bone* get_bone(std::string_view bone_name) const
		{
			return get_bone_by_name(bone_name);
		}

		const Bone* get_or_create_bone(std::string_view bone_name);
		const Bone* get_or_create_bone(BoneID bone_id);

		// Attempts to add a bone to this skeleton, returning a pointer to the internally stored object.
		// 
		// If the `bone_id` specified has already been added, this will
		// return a pointer to an existing bone, rather than emplacing a new one.
		const Bone* add_bone
		(
			BoneID bone_id,
			const math::Matrix& node_transform,
			const math::Matrix& offset,
			BoneID parent_bone_id={}
		);

		// Attempts to add a bone to this skeleton, returning a pointer to the internally stored object.
		// 
		// If the `bone_id` specified has already been added, this will
		// return a pointer to an existing bone, rather than emplacing a new one.
		const Bone* add_bone
		(
			BoneID bone_id,
			BoneID parent_bone_id={}
		);

		// Attempts to retrieve the index of the `bone` specified.
		// If `bone` does not originate from this skeleton, this will return an empty optional.
		std::optional<BoneIndex> get_bone_index(const Bone* bone) const;

		// Attempts to retrieve the index of the `bone_name` specified by computing a hash to act as an identifier.
		std::optional<BoneIndex> get_bone_index(std::string_view bone_name) const;

		// Attempts to retrieve the index of the `bone_id` specified.
		std::optional<BoneIndex> get_bone_index(BoneID bone_id) const;

		// Returns true if there are no bones in this skeleton.
		inline bool empty() const
		{
			return bones.empty();
		}

		// Returns true if this skeleton contains at least one bone.
		inline bool exists() const
		{
			return !empty();
		}

		inline std::size_t size() const
		{
			return bones.size();
		}

		// Equivalent to `exists`.
		inline explicit operator bool() const
		{
			return exists();
		}

		// The bones owned by this skeleton.
		BoneContainer bones;
	};
}