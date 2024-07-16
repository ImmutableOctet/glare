#include "application.hpp"
#include "window.hpp"

#include <third-party/lib.hpp>

#include <cassert>
#include <sdl2/SDL_events.h>

// Debugging related:
#include <iostream>
#include <util/log.hpp>

namespace app
{
	Application::Application(UpdateRate update_rate) :
		update_rate(update_rate),
		fixed_update_interval(static_cast<decltype(fixed_update_interval)>(1000 / update_rate))
	{
		assert(glare::lib::init_sdl());
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
			window = std::make_unique<app::Window>(width, height, title, flags);
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

			const auto frame_time = milliseconds();

			update(frame_time); update_counter++;

			handle_fixed_update(frame_time);

			begin_render(frame_time);
			render(frame_time); render_counter++;
			end_render(frame_time);
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

		on_execute();

		std::cout << "Running...\n";

		run();

		std::cout << "Shutting down...\n";

		stop();

		on_shutdown();

		std::cout << "Done.\n";
	}

	void Application::handle_fixed_update(Milliseconds frame_time)
	{
		// Execute the fixed update routine as many times
		// as needed to meet the intended interval.
		while (true)
		{
			const auto fixed_update_elapsed = (frame_time - fixed_update_timer);

			if (fixed_update_elapsed < fixed_update_interval)
			{
				break;
			}

			fixed_update(frame_time);
			fixed_update_counter++;

			//fixed_update_timer = frame_time;
			fixed_update_timer += fixed_update_interval;
		}
	}

	// Empty implementations:
	void Application::begin_render(Milliseconds time) {}
	void Application::render(Milliseconds time) {}
	void Application::end_render(Milliseconds time) {}
	
	entt::dispatcher* Application::get_event_handler() { return nullptr; }

	void Application::on_keydown(const keyboard_event_t& event) {}
	void Application::on_keyup(const keyboard_event_t& event) {}
	void Application::on_window_resize(Window& window, int width, int height) {}

	bool Application::process_event(const SDL_Event& e)
	{
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

	void Application::on_execute()
	{
		// Empty implementation.
	}

	void Application::on_shutdown()
	{
		// Empty implementation.
	}

	void Application::fixed_update(Milliseconds time)
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