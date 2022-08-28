#pragma once

namespace engine
{
	class Service;

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