#include "skeleton.hpp"

#include <assimp/mesh.h>

#include <util/string.hpp>

namespace graphics
{
	/*
	const Bone* Skeleton::get_bone(index_t bone_index) const
	{
		if ((bone_index > 0) && (bone_index < bones.size()))
		{
			return nullptr;
		}

		return &bones[bone_index];
	}
	*/

	const Bone* Skeleton::get_bone(index_t bone_index) const
	{
		for (const auto& kv : bones)
		{
			if (kv.second.id == bone_index)
			{
				return &kv.second;
			}
		}

		return nullptr;
	}

	/*
	const Bone* Skeleton::get_bone(std::string_view name) const
	{
		for (const auto& bone : bones)
		{
			if (bone.name == name)
			{
				return &bone;
			}
		}

		return nullptr;
	}
	*/

	const Bone* Skeleton::get_bone(std::string_view name) const
	{
		auto value_it = bones.find(std::string(name));

		if (value_it != bones.end())
		{
			const auto& value = *value_it;

			return &value.second;
		}

		return nullptr;
	}

	const Bone* Skeleton::get_or_create_bone(std::string_view name)
	{
		return add_bone(name);
	}

	const Bone* Skeleton::get_bone(const aiBone* bone_raw) const
	{
		return get_bone(util::to_string_view(bone_raw->mName));
	}

	const Bone* Skeleton::add_bone(std::string_view name, const math::Matrix& node_transform, const math::Matrix& offset, const std::string& parent_bone_name)
	{
		if (auto* bone = get_bone(name))
		{
			return bone;
		}

		BoneID bone_id = static_cast<BoneID>(bones.size());

		return &(bones[std::string(name)] = Bone{ .id=bone_id, .parent_name=parent_bone_name, .node_transform=node_transform, .offset=offset });
	}

	const Bone* Skeleton::add_bone(std::string_view name, const std::string& parent_bone_name)
	{
		return add_bone(name, math::identity_matrix(), math::identity_matrix(), parent_bone_name);
	}

	const Bone* Skeleton::add_bone(const aiBone* bone_raw, const math::Matrix& node_transform, const std::string& parent_bone_name)
	{
		return add_bone
		(
			util::to_string_view(bone_raw->mName),
			node_transform,
			//math::to_matrix(bone_raw->mOffsetMatrix),
			{},
			//(math::to_matrix(bone_raw->mOffsetMatrix) * glm::inverse(node_transform)),
			//(node_transform * math::to_matrix(bone_raw->mOffsetMatrix)),
			parent_bone_name
		);
	}

	/*
	Skeleton::index_t Skeleton::get_index(const Bone* bone_ptr, bool compare_name=false) const
	{
		for (index_t i = 0; i < bones.size(); i++)
		{
			const auto& bone = bones[i];

			if (compare_name)
			{
				if (bone.name == bone_ptr->name)
				{
					return i;
				}
			}
			else
			{
				//std::ptrdiff_t p = (...);

				if ((&bone) == bone_ptr)
				{
					return i;
				}
			}
		}

		return -1;
	}
	*/

	Skeleton::index_t Skeleton::get_index(const Bone* bone_ptr) const
	{
		return bone_ptr->id;
	}
}