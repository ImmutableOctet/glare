#pragma once

#include "types.hpp"
#include "delta_time.hpp"
#include "input/input.hpp"
#include "timer.hpp"

#include <entt/signal/dispatcher.hpp>

#include <string_view>
#include <atomic>
#include <chrono>

// SDL:
struct SDL_KeyboardEvent;
union SDL_Event;

namespace app
{
	class Window;

	class Application : public TimedEventManager
	{
		public:
			using UpdateRate = DeltaTime::Rate;

			using time_point = std::chrono::time_point<std::chrono::system_clock>;
			using clock_duration = std::chrono::system_clock::duration;

			using keyboard_event_t = SDL_KeyboardEvent;
		protected:
			friend Window;

			bool running : 1 = false;

			std::unique_ptr<Window> window;

			std::chrono::time_point<std::chrono::system_clock> start_point;
			std::chrono::time_point<std::chrono::system_clock> stop_point;

			std::uint64_t update_counter = 0;
			std::uint64_t render_counter = 0;

			//DeltaTime delta_time;

			// TODO: Migrate this field to the `Game` class.
			input::InputHandler input;

			UpdateRate fixed_update_rate;
		protected:
			Application(UpdateRate update_rate);
			virtual ~Application() = default;

			Window& make_window(int width, int height, std::string_view title="", WindowFlags flags=WindowFlags::Default);

			time_point now() const;
			clock_duration time() const;
			
			std::int64_t unix_time() const;
			std::int64_t milliseconds() const;

			inline Duration fixed_update_duration() const
			{
				return (1000 / fixed_update_rate);
			}

			virtual void on_execute();

			virtual void fixed_update(Milliseconds time);

			virtual void update(Milliseconds time) abstract; // const DeltaTime& delta_time

			virtual void render() abstract;
			virtual void begin_render();
			virtual void end_render();

			// Optional; empty implementation.
			virtual entt::dispatcher* get_event_handler();

			virtual void on_keydown(const keyboard_event_t& event);
			virtual void on_keyup(const keyboard_event_t& event);
			virtual void on_window_resize(Window& window, int width, int height);

			// Starts all additional threads associated with the application.
			bool start();

			// Executes the main application thread.
			void run();

			// Stops all additional threads associated with the application.
			// To ensure proper finalization, this should be executed after calling 'run'.
			bool stop();
		public:
			inline bool is_running() const { return running; }
			inline Window* get_window() const { return window.get(); }

			virtual bool handle_events();

			// Returning `true` indicates that the event was handled, and will therefore
			// not need to be processed further by the default implementation of `handle_events`.
			virtual bool process_event(const SDL_Event& e);

			void execute();
		private:
			void retrieve_start_time();
			void retrieve_stop_time();
	};
}