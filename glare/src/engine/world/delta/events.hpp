#pragma once

#include "types.hpp"

namespace engine
{
	struct LifetimeDeltaComponent;

	struct OnDeltaSnapshot
	{
		DeltaTimestamp snapshot_timestamp = {};
		DeltaTimestamp latest_delta_timestamp = {};

		float latest_delta = 1.0f;
	};
	
	struct DeltaLifetimeEvent
	{
		Entity entity = null;
		MetaTypeID component_type_id = {};

		const LifetimeDeltaComponent* lifetime = {};
	};

	// Triggered when a tracked component's lifetime begins.
	struct OnDeltaLifetimeBegin : DeltaLifetimeEvent {};

	// Triggered when a tracked component's lifetime comes to an end.
	struct OnDeltaLifetimeEnd : DeltaLifetimeEvent {};

	// Triggered when a tracked component's lifetime begins again,
	// after having already ended in the same snapshot.
	struct OnDeltaLifetimeRestart : DeltaLifetimeEvent {};
}