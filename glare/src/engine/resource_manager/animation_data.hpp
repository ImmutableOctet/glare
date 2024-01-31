#pragma once

#include <engine/types.hpp>

#include <engine/world/animation/types.hpp>
#include <engine/world/animation/animation_slice.hpp>
#include <engine/world/animation/skeletal_frame_data.hpp>
#include <engine/world/animation/skeleton.hpp>

//#include <engine/world/animation/animation_repository.hpp>

#include <unordered_map>

namespace engine
{
	struct AnimationData
	{
		using SliceContainer = std::unordered_map<AnimationID, AnimationSlice>; // AnimationRepository::SliceContainer;
		using AnimationContainer = SliceContainer;

		Skeleton skeleton;
		SkeletalFrameData frames;
		AnimationContainer animations;
	};
}