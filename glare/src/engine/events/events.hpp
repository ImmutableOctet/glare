#pragma once

#include "input.hpp"

namespace engine
{
	class World;

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