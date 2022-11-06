#pragma once

//#include "types.hpp"

namespace engine
{
	class Service;

	// Helper for adding a `service` field to an event type.
	// 
	// NOTE: Not every event type with a `service` field inherits from this type,
	// this is simply an easy way to add the field inherently.
	struct ServiceOriginatedEvent
	{
		Service* service;
	};
}