#pragma once

#include <engine/types.hpp>
#include <graphics/types.hpp>

#include <engine/world/animation/skeletal_frame_data.hpp>
#include <engine/world/animation/skeleton.hpp>
#include <engine/world/animation/animation_repository.hpp>

namespace engine
{
	struct AnimationData
	{
		Skeleton skeleton;
		SkeletalFrameData frames;
		AnimationRepository animations;
	};
}