#pragma once

//#include <engine/events.hpp>
//#include <engine/world/events.hpp>

namespace graphics
{
	struct Animation;
}

namespace engine
{
	class World;
	struct Animator;
	struct TransformComponent;

	/*
		TODO:
			* Ensure that usage of pointer-to-component does not cause any unintended side effects.
			(e.g. pointer to moved/reallocated component could cause problems if events are queued)
	*/

	// Triggered any time an animation completes.
	// (Happens repeatedly in the case of looping animations)
	struct OnAnimationComplete
	{
		Entity entity;

		const Animator* animator;
		const Animation* animation;
	};

	struct OnAnimationChange
	{
		Entity entity;

		const Animator* animator;
		const Animation* prev_animation;
		const Animation* current_animation;

		float transition_length = 0.0f;
	};

	// Executed each tick/frame an entity's animation updates.
	struct OnAnimationFrame
	{
		Entity entity;

		float current_time;
		float prev_time;

		const Animator* animator;
		const Animation* current_animation;

		std::uint16_t bones_changed;
	};
}