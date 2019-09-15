#pragma once

#include <string>
#include <atomic>
#include <chrono>

#include "types.hpp"
#include <util/lib.hpp>

#include "input/input_handler.hpp"

namespace app
{
	class Window;
	class EventHandler;

	class Application
	{
		public:
			using time_point = std::chrono::time_point<std::chrono::system_clock>;
			using duration = std::chrono::system_clock::duration;
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
			std::atomic<bool> running = false; // <-- May change later. (Could use a mutex for this, etc)

			std::chrono::time_point<std::chrono::system_clock> start_point;
			std::chrono::time_point<std::chrono::system_clock> stop_point;

			//input::InputHandler input;
		protected:
			Application();

			Window& make_window(int width, int height, const std::string& title="", WindowFlags flags=WindowFlags::Default);

			time_point now() const;
			duration time() const;
			
			std::int64_t unix_time() const;
			std::int64_t milliseconds() const;

			virtual void update() abstract;
			virtual void render() abstract;
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

			inline void execute()
			{
				start();
				run();
				stop();
			}
		private:
			void retrieve_start_time();
			void retrieve_stop_time();
	};
}