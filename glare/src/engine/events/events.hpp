#pragma once

#include <string>
#include <optional>
#include <filesystem>

#include <engine/types.hpp>
#include "input.hpp"

namespace graphics
{
	struct Animation;
}

namespace engine
{
	class World;
	struct Animator;
	struct TransformComponent;

	//template <EntityType type>
	struct OnEntityCreated
	{
		//static constexpr auto Type = type;

		inline EntityType get_type() const { return type; }

		Entity entity;
		Entity parent;
		EntityType type;
	};

	//template <EntityType type>
	struct OnEntityDestroyed
	{
		//static constexpr auto Type = type;
		inline EntityType get_type() const { return type; }

		Entity entity;
		Entity parent;
		EntityType type;

		bool destroy_orphans = true;
	};

	struct OnParentChanged
	{
		Entity entity, from_parent, to_parent;
	};

	template <typename ComponentType>
	struct OnComponentAdd
	{
		using Type = ComponentType;

		/*
		OnComponentAdd(const OnComponentAdd&) = default;
		OnComponentAdd& operator=(const OnComponentAdd&) = default;

		OnComponentAdd(OnComponentAdd&&) = default;
		OnComponentAdd& operator=(OnComponentAdd&&) = default;
		*/

		//World& world;
		//ComponentType& component;
		Entity entity;
	};

	template <typename ComponentType>
	struct OnComponentChange
	{
		using Type = ComponentType;

		//World& world;
		//ComponentType& component;
		Entity entity;
	};

	// Triggered any time an animation completes.
	// (Happens repeatedly in the case of looping animations)
	struct OnAnimationComplete
	{
		Entity entity;

		const Animator*  animator;
		const Animation* animation;
	};

	struct OnAnimationChange
	{
		Entity entity;

		const Animator*  animator;
		const Animation* prev_animation;
		const Animation* current_animation;
	};

	// Executed each tick/frame an entity's animation updates.
	struct OnAnimationFrame
	{
		Entity entity;

		float current_time;
		float prev_time;

		const Animator*  animator;
		const Animation* current_animation;
	};

	struct OnStageLoaded
	{
		Entity root;
		std::optional<std::filesystem::path> path = std::nullopt;
	};

	using OnTransformChange = OnComponentChange<TransformComponent>;
}