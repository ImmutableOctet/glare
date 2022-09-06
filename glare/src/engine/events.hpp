#pragma once

/*
	Standard game engine event types.

	System-specific event types can usually be found
	in `events` submodules for each system.
*/

#include "types.hpp"

namespace engine
{
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

		//Service& service;
		//ComponentType& component;
		Entity entity;
	};

	/*
		Helper template for simple "this entity changed" events, where the
		relevant data is essentially the statically associated type.

		e.g. `OnTransformChange` is just `OnComponentChange<TransformComponent>`,
		since all of the needed data can be found in the component itself.
	*/
	/*
	// TODO: Revisit this concept.
	template <typename ComponentType>
	struct OnComponentChange
	{
		using Type = ComponentType;

		//Service& service;
		//ComponentType& component;
		Entity entity;
	};
	*/
}