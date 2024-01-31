#pragma once

#include <engine/world/animation/animation_state.hpp>
#include <engine/world/animation/animation_transition.hpp>

namespace engine
{
	struct AnimationTransitionComponent
	{
		// The ongoing transition between `AnimationComponent::current`
		// and `AnimationComponent::previous`.
		AnimationTransition transition;
	};
}