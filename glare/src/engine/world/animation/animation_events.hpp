#pragma once

#include <engine/types.hpp>

//#include <engine/events.hpp>
//#include <engine/world/events.hpp>

namespace graphics
{
	struct AnimationData;
}

namespace engine
{
	class World;

	struct OnAnimationLayerActivated
	{
		AnimationID animation = {};
	};

	struct OnAnimationLayerDeactivated
	{
		AnimationID animation = {};
	};

	// Triggered any time an animation completes.
	// (Happens repeatedly in the case of looping animations)
	/*
	struct OnAnimationComplete
	{
		Entity entity = null;
		
		AnimationID current = {};
		AnimationID previous = {};
	};
	*/

	/*
	struct OnAnimationChange
	{
		Entity entity;
	};
	*/

	// Executed each tick/frame an entity's animation updates.
	/*
	struct OnAnimationFrame
	{
		Entity entity;

		float current_time;
		float prev_time;

		const AnimationComponent* animator;
		const AnimationData* current_animation;

		std::uint16_t bones_changed;
	};
	*/
}