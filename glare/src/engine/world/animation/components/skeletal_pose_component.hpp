#pragma once

#include <math/types.hpp>

//#include <util/small_vector.hpp>
#include <vector>

namespace engine
{
	struct SkeletalPoseComponent
	{
		using Matrices = std::vector<math::Matrix>; // util::small_vector<32, math::Matrix>

		// Buffer containing the last updated state of each bone.
		Matrices pose; // bone_matrices;

		inline const std::vector<math::Matrix>& get_pose() const
		{
			return pose;
		}

		inline std::size_t count_bones() const
		{
			return pose.size();
		}
	};
}