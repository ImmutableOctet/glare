#include "application.hpp"

#include "window.hpp"

namespace app
{
	Application::Application() : init_libraries() {}

	bool Application::start()
	{
		//events.start();

		running = true;

		return is_running();
	}

	bool Application::stop()
	{
		//events.stop();

		running = false;

		return !is_running();
	}

	Window& Application::make_window(int width, int height, const std::string& title)
	{
		if (window == nullptr)
		{
			window = memory::unique<app::Window>(width, height, title);
		}

		return (*window);
	}

	void Application::run()
	{
		while (is_running())
		{
			if (!window->handle_events())
			{
				stop();

				break;
			}

			update();
			render();
		}
	}
}