#pragma once

#include "service_originated_event.hpp"

#include <app/types.hpp>

namespace app
{
	struct Graphics;
}

namespace engine
{
	class Service;

	//struct ServiceOriginatedEvent;

	// This event is triggered immediately by a `Service`, once per continuous update.
	struct OnServiceUpdate : public ServiceOriginatedEvent
	{
		// A snapshot of the application's up-time in milliseconds.
		app::Milliseconds time;

		// A normalized rate of execution. This represents the amount
		// of time that has passed since the last `OnServiceUpdate` trigger.
		float delta = 1.0f;
	};

	// This event is triggered immediately by a `Service`, once per fixed update tick.
	struct OnServiceFixedUpdate : public ServiceOriginatedEvent
	{
		// A snapshot of the application's up-time in milliseconds.
		app::Milliseconds time;

		// Placeholder delta value. (Should always be `1.0f`)
		float delta = 1.0f;
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