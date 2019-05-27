#include "application.hpp"

namespace app
{
	Application::Application()
		: events(*this, false) {}

	bool Application::start()
	{
		events.start();

		running = true;

		return is_running();
	}

	bool Application::stop()
	{
		events.stop();

		running = false;

		return !is_running();
	}

	void Application::run()
	{
		while (is_running())
		{
			update();
			render();
		}
	}
}