#include "application.hpp"

#include "window.hpp"

namespace app
{
	Application::Application() : init_libraries() {}

	bool Application::start()
	{
		//events.start();

		running = true;

		retrieve_start_time();

		return is_running();
	}

	bool Application::stop()
	{
		//events.stop();

		running = false;

		retrieve_stop_time();

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

	Application::time_point Application::now() const
	{
		return std::chrono::system_clock::now();
	}

	Application::duration Application::time() const
	{
		return now().time_since_epoch();
	}

	std::int64_t Application::unix_time() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(time()).count();
	}

	std::int64_t Application::milliseconds() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(now() - start_point).count();
	}

	void Application::retrieve_start_time()
	{
		start_point = now();
	}

	void Application::retrieve_stop_time()
	{
		stop_point = now();
	}
}