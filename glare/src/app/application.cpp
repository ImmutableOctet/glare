#include <sdl2/SDL_events.h>
#include <iostream>

#include "application.hpp"
#include "window.hpp"

namespace app
{
	Application::Application(UpdateRate update_rate) :
		init_libraries(),
		//delta_time(update_rate),
		fixed_update_rate(update_rate)
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

	Window& Application::make_window(int width, int height, std::string_view title, WindowFlags flags)
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

			auto time = milliseconds();

			// Update timed events.
			*this << time;

			update(time); update_counter++;

			begin_render();
			render(); render_counter++;
			end_render();
		}
	}

	bool Application::handle_events()
	{
		SDL_Event e;
		
		//int events_processed = 0;

		while (SDL_PollEvent(&e)) // SDL_WaitEvent
		{
			if (process_event(e))
			{
				//events_processed++;

				continue;
			}

			switch (e.type)
			{
				case SDL_QUIT:
					return false;

				case SDL_WINDOWEVENT:
				{
					const auto& window_event = e.window;

					if (window->get_id() == window_event.windowID)
					{
						if (!window->process_event(e, window_event, *this))
						{
							return false;
						}
					}

					break;
				}

				case SDL_KEYDOWN:
					on_keydown(e.key);

					break;

				case SDL_KEYUP:
					on_keyup(e.key);

					break;

				default:
					continue;
			}

			//events_processed++;
		}

		//return events_processed;
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

	// Empty implementations:
	void Application::begin_render() {}
	void Application::end_render() {}
	entt::dispatcher* Application::get_event_handler() { return nullptr; }
	void Application::on_keydown(const keyboard_event_t& event) {}
	void Application::on_keyup(const keyboard_event_t& event) {}
	void Application::on_window_resize(Window& window, int width, int height) {}

	bool Application::process_event(const SDL_Event& e)
	{
		// TODO: Implement method of passing an event-handler object to `input.process_event`.
		if (input.process_event(e, get_event_handler()))
		{
			return true;
		}

		return false;
	}

	// Date/time related:
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