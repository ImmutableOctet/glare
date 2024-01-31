#include "skeleton.hpp"

#include <engine/meta/hash.hpp>

#include <math/math.hpp>
#include <util/string.hpp>

#include <assimp/mesh.h>

#include <cassert>

namespace engine
{
	const Bone* Skeleton::get_bone_by_index(BoneIndex bone_index) const
	{
		assert(bone_index >= 0);

		const auto bone_index_converted = static_cast<std::size_t>(bone_index);

		if ((bone_index_converted < bones.size()))
		{
			return nullptr;
		}

		return &(bones[bone_index_converted]);
	}

	const Bone* Skeleton::get_bone_by_id(BoneID bone_id) const
	{
		for (const auto& bone : bones)
		{
			if (bone.name == bone_id)
			{
				return &bone;
			}
		}

		return {};
	}

	const Bone* Skeleton::get_bone_by_name(std::string_view bone_name) const
	{
		if (bone_name.empty())
		{
			return {};
		}

		const auto bone_id = hash(bone_name);

		return get_bone_by_id(bone_id);
	}

	const Bone* Skeleton::get_or_create_bone(std::string_view bone_name)
	{
		const auto bone_id = hash(bone_name);

		return get_or_create_bone(bone_id);
	}

	const Bone* Skeleton::get_or_create_bone(BoneID bone_id)
	{
		return add_bone(bone_id);
	}

	const Bone* Skeleton::add_bone
	(
		BoneID bone_id,
		const math::Matrix& node_transform,
		const math::Matrix& offset,
		BoneID parent_bone_id
	)
	{
		if (const auto bone = get_bone_by_id(bone_id))
		{
			return bone;
		}

		const auto& bone_out = bones.emplace_back
		(
			bone_id,
			parent_bone_id,
			node_transform,
			offset
		);

		return &bone_out;
	}

	const Bone* Skeleton::add_bone
	(
		BoneID bone_id,
		BoneID parent_bone_id
	)
	{
		return add_bone(bone_id, math::identity_matrix(), math::identity_matrix(), parent_bone_id);
	}

	std::optional<BoneIndex> Skeleton::get_bone_index(const Bone* bone) const
	{
		if (!bone)
		{
			return std::nullopt;
		}

		const auto bones_begin = bones.data();
		const auto bones_end   = (bones.data() + bones.size());

		if ((bone < bones_begin) || (bone >= bones_end))
		{
			return std::nullopt;
		}

		return static_cast<BoneIndex>(bone - bones_begin);
	}

	std::optional<BoneIndex> Skeleton::get_bone_index(std::string_view bone_name) const
	{
		if (const auto bone = get_bone_by_name(bone_name))
		{
			return get_bone_index(bone);
		}

		return std::nullopt;
	}

	std::optional<BoneIndex> Skeleton::get_bone_index(BoneID bone_id) const
	{
		if (const auto bone = get_bone_by_id(bone_id))
		{
			return get_bone_index(bone);
		}

		return std::nullopt;
	}

	/*
	const Bone* Skeleton::add_bone
	(
		const aiBone& bone_raw,
		const math::Matrix& node_transform,

		std::string_view parent_bone_name
	)
	{
		return add_bone
		(
			hash(util::to_string_view(bone_raw.mName)),

			node_transform,
			
			//math::to_matrix(bone_raw->mOffsetMatrix),
			//(math::to_matrix(bone_raw->mOffsetMatrix) * glm::inverse(node_transform)),
			//(node_transform * math::to_matrix(bone_raw->mOffsetMatrix)),
			{},

			hash(parent_bone_name)
		);
	}
	*/

	/*
	const Bone* Skeleton::get_bone(const aiBone* bone_raw) const
	{
		return get_bone_by_name(util::to_string_view(bone_raw->mName));
	}
	*/
}