#pragma once

#include <thread>
#include <mutex>

#include <types.hpp>

#include "input/input_handler.hpp"

namespace app
{
	class Application;

	// Asynchronous event handler. (Base class)
	class EventHandler // abstract
	{
		public:
			using lock_t = std::lock_guard<std::mutex>;

			EventHandler(Application& app_inst, bool auto_start=false);
			~EventHandler();

			void start();
			void stop(bool join_thread=true);

			lock_t&& pause();
		protected:
			void poll();

			virtual void run() abstract;

			std::thread event_thread;
			std::mutex event_mutex;

			Application& app_instance;

			bool running = false;
	};
}