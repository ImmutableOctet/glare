#pragma once

#include <string>
#include <atomic>
#include <chrono>

#include <types.hpp>
#include <util/lib.hpp>

#include "input/input_handler.hpp"

namespace app
{
	class Window;
	class EventHandler;

	class Application
	{
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

			//input::InputHandler input;

			Application();

			Window& make_window(int width, int height, const std::string& title="");

			inline std::int64_t milliseconds() const
			{
				auto now = std::chrono::system_clock::now();
				auto duration = now.time_since_epoch();

				return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
			}

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

			inline void execute()
			{
				start();
				run();
				stop();
			}
	};
}