#include <sdl2/SDL_events.h>
#include <iostream>

#include "application.hpp"
#include "window.hpp"

namespace app
{
	Application::Application(UpdateRate update_rate) : init_libraries(), delta_time(update_rate), fixed_update_rate(update_rate)
	{
		auto time = milliseconds();

		*this << Timer::make_continuous(fixed_update_duration(), time, [this](Timer& timer, Duration time_elapsed)
		{
			fixed_update();

			return fixed_update_duration();
		});
	}

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
		auto time = milliseconds();

		while (is_running())
		{
			if (!handle_events())
			{
				stop();

				break; // return;
			}

			time = milliseconds();

			// Update timed events.
			*this << time;

			// Update the delta-timer.
			delta_time << time;

			update(delta_time); update_counter++;
			render(); render_counter++;
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
				{
					const auto& window_event = event.window;

					if (window->get_id() == window_event.windowID)
					{
						if (!window->handle_event(event, window_event))
						{
							return false;
						}
					}

					break;
				}
				case SDL_KEYDOWN:
					on_keydown(event.key);

					break;
				case SDL_KEYUP:
					on_keyup(event.key);

					break;
			}
		}

		return true;
	}

	void Application::execute()
	{
		std::cout << "Executing start-up procedure...\n";

		start();

		std::cout << "Running...\n";

		run();

		std::cout << "Shutting down...\n";

		stop();

		std::cout << "Done.\n";
	}

	void Application::on_keydown(const keyboard_event_t& event)
	{
	}

	void Application::on_keyup(const keyboard_event_t& event)
	{
		
	}

	Application::time_point Application::now() const
	{
		return std::chrono::system_clock::now();
	}

	Application::clock_duration Application::time() const
	{
		return now().time_since_epoch();
	}

	Milliseconds Application::unix_time() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(time()).count();
	}

	Milliseconds Application::milliseconds() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(now() - start_point).count();
	}

	void Application::fixed_update()
	{
		// Empty implementation.
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