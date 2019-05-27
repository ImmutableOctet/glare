#pragma once

#include <thread>
#include <mutex>

namespace app
{
	class Application;

	class EventHandler
	{
		public:
			using lock_t = std::lock_guard<std::mutex>;

			EventHandler(Application& app_inst, bool auto_start=false);
			~EventHandler();

			void start();
			void stop(bool join_thread=true);

			lock_t&& pause();
		protected:
			void run();

			std::thread event_thread;
			std::mutex event_mutex;

			Application& app_instance;

			bool running = false;
	};
}