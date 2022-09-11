#pragma once

namespace app
{
	struct Graphics;
}

namespace engine
{
	class Service;

	// This event is triggered immediately by a `Service`, once per update tick.
	struct OnServiceUpdate
	{
		Service* service;

		// A normalized rate of execution. This represents the amount
		// of time that has passed since the last `OnServiceUpdate` trigger.
		float delta;
	};

	// This event is triggered immediately by a `Service`, once per frame/render, if applicable.
	struct OnServiceRender
	{
		Service* service;
		app::Graphics* graphics;
	};

	// TODO: Implement pause events.
	struct OnServicePause
	{
		Service* service;
	};

	// TODO: Implement resume events.
	struct OnServiceResume
	{
		Service* service;
	};
}