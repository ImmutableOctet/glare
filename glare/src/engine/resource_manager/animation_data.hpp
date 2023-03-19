#pragma once

#include <engine/types.hpp>
#include <graphics/types.hpp>

#include <graphics/animation.hpp>
#include <graphics/skeleton.hpp>

#include <vector>
#include <map>
#include <tuple>

namespace engine
{
	using Animations = std::vector<graphics::Animation>;
	using AnimationTransitions = std::map<std::tuple<AnimationID, AnimationID>, float>;

	struct AnimationData
	{
		using ID = AnimationID;

		graphics::Skeleton skeleton;
		Animations animations;

		// Mapping of to/from animations to a corresponding interpolation duration in frames.
		AnimationTransitions transitions;

		float get_transition(AnimationID src, AnimationID dest) const;
	};
}