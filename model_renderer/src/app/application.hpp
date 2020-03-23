#pragma once

#include <string>
#include <atomic>
#include <chrono>

#include "types.hpp"
#include <util/lib.hpp>

#include "delta_time.hpp"
#include "input/input.hpp"

// SDL:
struct SDL_KeyboardEvent;

namespace app
{
	class Window;
	class EventHandler;

	class Application
	{
		public:
			using time_point = std::chrono::time_point<std::chrono::system_clock>;
			using duration = std::chrono::system_clock::duration;

			using keyboard_event_t = SDL_KeyboardEvent;
		protected:
			friend EventHandler;

			struct Libraries
			{
				Libraries()
				{
					ASSERT(util::lib::init_sdl());
				}

				~Libraries() {}
			};

			Libraries init_libraries;
			unique_ref<Window> window;

			// Atomic so that we can see if the application is running from another thread.
			bool running = false; // <-- May change later. (Could use a mutex for this, etc) // std::atomic<bool>

			std::chrono::time_point<std::chrono::system_clock> start_point;
			std::chrono::time_point<std::chrono::system_clock> stop_point;

			std::uint64_t update_counter = 0;
			std::uint64_t render_counter = 0;

			DeltaTime delta_time;
			input::InputHandler input;
		protected:
			Application(DeltaTime::Rate update_rate);

			Window& make_window(int width, int height, const std::string& title="", WindowFlags flags=WindowFlags::Default);

			time_point now() const;
			duration time() const;
			
			std::int64_t unix_time() const;
			std::int64_t milliseconds() const;

			virtual void update(const DeltaTime& delta_time) abstract;
			virtual void render() abstract;

			virtual void on_keydown(const keyboard_event_t& event);
			virtual void on_keyup(const keyboard_event_t& event);
		public:
			inline bool is_running() const { return running; }

			// Starts all additional threads associated with the application.
			bool start();

			// Executes the main application thread.
			void run();

			// Stops all additional threads associated with the application.
			// To ensure proper finalization, this should be executed after calling 'run'.
			bool stop();

			bool handle_events();

			void execute();
		private:
			void retrieve_start_time();
			void retrieve_stop_time();
	};
}