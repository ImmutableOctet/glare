#pragma once

#include <math/types.hpp>

//#include <util/small_vector.hpp>
#include <vector>

namespace engine
{
	struct SkeletalPose
	{
		// Vector of matrices by bone index.
		using Container = std::vector<math::Matrix>; // util::small_vector<32, math::Matrix>

		// Buffer containing the transformation of each bone by its index.
		Container bone_matrices;

		inline const SkeletalPose::Container& get_pose() const
		{
			return bone_matrices;
		}

		inline std::size_t size() const
		{
			return bone_matrices.size();
		}
	};
}