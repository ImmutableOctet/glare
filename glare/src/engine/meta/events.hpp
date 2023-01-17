#pragma once

#include "types.hpp"

namespace engine
{
	struct ComponentEvent
	{
		// The entity whose component we're referencing.
		Entity entity;

		// Opaque reference to the updated component.
		MetaAny component;

		inline MetaType type() const
		{
			return component.type();
		}

		inline explicit operator bool() const
		{
			return static_cast<bool>(component);
		}
	};

	struct OnComponentCreate  : public ComponentEvent {};
	struct OnComponentUpdate  : public ComponentEvent {};
	struct OnComponentDestroy : public ComponentEvent {};
}