#pragma once

#include "types.hpp"
#include "window_flags.hpp"
#include "delta_time.hpp"
#include "input/input.hpp"

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

	class Application
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

			// TODO: Migrate this field to the `Game` class.
			input::InputHandler input;
			
			Application(UpdateRate update_rate);

			virtual ~Application() = default;

			Window& make_window(int width, int height, std::string_view title="", WindowFlags flags=WindowFlags::Default);

			time_point now() const;
			clock_duration time() const;
			
			std::int64_t unix_time() const;
			std::int64_t milliseconds() const;

			virtual void on_execute();
			virtual void on_shutdown();

			virtual bool handle_events();

			virtual void fixed_update(Milliseconds time);

			virtual void update(Milliseconds time) abstract; // const DeltaTime& delta_time

			virtual void render(Milliseconds time);
			virtual void begin_render(Milliseconds time);
			virtual void end_render(Milliseconds time);

			// Optional; empty implementation.
			virtual entt::dispatcher* get_event_handler();

			virtual void on_keydown(const keyboard_event_t& event);
			virtual void on_keyup(const keyboard_event_t& event);
			virtual void on_window_resize(Window& window, int width, int height);

			// Returning `true` indicates that the event was handled, and will therefore
			// not need to be processed further by the default implementation of `handle_events`.
			virtual bool process_event(const SDL_Event& e);

			// Starts all additional threads associated with the application.
			bool start();

			// Executes the main application thread.
			void run();

			// Stops all additional threads associated with the application.
			// To ensure proper finalization, this should be executed after calling 'run'.
			bool stop();

		private:
			UpdateRate update_rate;

			std::uint64_t update_counter       = 0;
			std::uint64_t fixed_update_counter = 0;
			std::uint64_t render_counter       = 0;

			Duration fixed_update_interval = {};
			app::Milliseconds fixed_update_timer = {};

			void retrieve_start_time();
			void retrieve_stop_time();

			void handle_fixed_update(Milliseconds frame_time);

		public:
			void execute();

			inline bool is_running() const { return running; }
			
			inline Window* get_window() const { return window.get(); }

			inline UpdateRate get_update_rate() const { return update_rate; }

			inline std::uint64_t get_update_counter() const { return update_counter; }
			inline std::uint64_t get_fixed_update_counter() const { return fixed_update_counter; }
			inline std::uint64_t get_render_counter() const { return render_counter; }
	};
}