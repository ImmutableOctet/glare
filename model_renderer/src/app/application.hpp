#pragma once

#include <string>
#include <atomic>

#include "event_handler.hpp"

namespace app
{
	class Window;

	class Application
	{
		protected:
			friend EventHandler;

			Application();

			// Atomic so that we can see if the application is running from another thread.
			std::atomic<bool> running = false; // <-- May change later. (Could use a mutex for this, etc)

			EventHandler events;

			bool start();
			bool stop();

			virtual void update() = 0;
			virtual void render() = 0;
		public:
			inline bool is_running() { return (running); }

			void run();
	};
}