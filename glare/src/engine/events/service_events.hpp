#pragma once

namespace engine
{
	class System;

	struct OnServiceUpdate
	{
		Service* service;
		float delta;
	};

	struct OnServicePause
	{
		Service* service;
	};

	struct OnServiceResume
	{
		Service* service;
	};
}