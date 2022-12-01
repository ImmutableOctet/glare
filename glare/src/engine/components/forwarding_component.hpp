#pragma once

#include "types.hpp"

namespace engine
{
	/*
		Allows an entity to specify another entity (e.g. object's 'root' parent/pivot entity)
		to handle logic meant for the 'full' object.

		Useful for cases where you have a child entity trigger some behavior that should
		involve everything tied to the object; e.g. motion events, zone triggers, etc.
	*/
	struct ForwardingComponent
	{
		Entity root_entity;
	};
}