#pragma once

#include "service_originated_event.hpp"

//#include <functional>

namespace app
{
	struct Graphics;
}

namespace engine
{
	class Service;

	//struct ServiceOriginatedEvent;

	// This event is triggered immediately by a `Service`, once per update tick.
	struct OnServiceUpdate : public ServiceOriginatedEvent
	{
		// A normalized rate of execution. This represents the amount
		// of time that has passed since the last `OnServiceUpdate` trigger.
		float delta;
	};

	// This event is triggered immediately by a `Service`, once per frame/render, if applicable.
	struct OnServiceRender : public ServiceOriginatedEvent
	{
		app::Graphics* graphics; // std::reference_wrapper<app::Graphics>
	};

	// TODO: Implement pause events.
	struct OnServicePause : public ServiceOriginatedEvent {};

	// TODO: Implement resume events.
	struct OnServiceResume : public ServiceOriginatedEvent {};
}