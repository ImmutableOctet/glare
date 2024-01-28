#pragma once

#include <engine/world/animation/skeletal_pose.hpp>

namespace engine
{
	struct SkeletalPoseComponent
	{
		// The last updated state of each bone.
		SkeletalPose pose;

		inline const SkeletalPose::Container& get_pose() const
		{
			return pose.get_pose();
		}

		inline std::size_t count_bones() const
		{
			return pose.size();
		}
	};
}