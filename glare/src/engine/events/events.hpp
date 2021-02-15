#pragma once

#include <engine/types.hpp>
#include "input.hpp"

namespace engine
{
	class World;

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
}