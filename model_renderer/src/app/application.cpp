#include <sdl2/SDL_events.h>

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

	Window& Application::make_window(int width, int height, const std::string& title, WindowFlags flags)
	{
		if (window == nullptr)
		{
			window = memory::unique<app::Window>(width, height, title, flags);
		}

		return (*window);
	}

	void Application::run()
	{
		while (is_running())
		{
			if (!handle_events())
			{
				stop();

				break; // return;
			}

			update();
			render();
		}
	}

	bool Application::handle_events()
	{
		SDL_Event event;

		while (SDL_PollEvent(&event)) // SDL_WaitEvent
		{
			switch (event.type)
			{
				case SDL_QUIT:
					return false;
				case SDL_WINDOWEVENT:
					const auto& window_event = event.window;

					if (window->get_id() == window_event.windowID)
					{
						if (!window->handle_event(event, window_event))
						{
							return false;
						}
					}

					break;
				case SDL_KEYDOWN:
					break;
			}
		}

		return true;
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