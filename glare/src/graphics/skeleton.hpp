#pragma once

//#include <vector>
#include <string>
#include <unordered_map>
#include <string_view>

#include <math/types.hpp>

#include "types.hpp"

struct aiBone;

namespace graphics
{
	struct Bone
	{
		using ID = BoneID;

		ID id;

		std::string parent_name;

		// Local matrix transform for the node containing this bone.
		math::Matrix node_transform;

		// Model-space to bone-local-space offset matrix.
		math::Matrix offset;

		//std::string name;
	};

	// A vector of offset matrices from the model's origin, each representing a bone at that index.
	//using Skeleton = std::vector<Bone>;

	struct Skeleton
	{
		using index_t = BoneID;

		using IndexType = index_t;
		using Container = std::unordered_map<std::string, Bone>; // std::vector<Bone>;

		const Bone* get_bone(index_t bone_index) const;
		const Bone* get_bone(std::string_view name) const;
		const Bone* get_or_create_bone(std::string_view name);

		const Bone* get_bone(const aiBone* bone_raw) const;

		const Bone* add_bone(std::string_view name, const math::Matrix& node_transform, const math::Matrix& offset, const std::string& parent_bone_name={});
		const Bone* add_bone(std::string_view name, const std::string& parent_bone_name={});

		//const Bone* add_bone(const aiBone* bone_raw);
		const Bone* add_bone(const aiBone* bone_raw, const math::Matrix& node_transform, const std::string& parent_bone_name={});

		// Returns `-1` on failure.
		index_t get_index(const Bone* bone_ptr) const;
		//index_t get_index(const Bone* bone_ptr, bool compare_name=false) const;

		bool empty() const { return bones.empty(); }
		bool exists() const { return !empty(); }

		inline operator bool() const { return exists(); }

		//std::size_t size() const { return bones.size(); }
		index_t size() const { return static_cast<index_t>(bones.size()); }

		Container bones;
	};
}