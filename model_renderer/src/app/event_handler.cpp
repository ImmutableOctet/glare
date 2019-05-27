#include "event_handler.hpp"
#include "application.hpp"

#include <sdl2/SDL_events.h>

namespace app
{
	EventHandler::EventHandler(Application& app_inst, bool auto_start)
		: app_instance(app_inst)
	{
		if (auto_start)
		{
			start();
		}
	}

	EventHandler::~EventHandler()
	{
		stop(true);
	}

	void EventHandler::start()
	{
		running = true;

		event_thread = std::thread(&EventHandler::run, this);
	}

	EventHandler::lock_t&& EventHandler::pause()
	{
		return std::move(lock_t(event_mutex));
	}

	void EventHandler::stop(bool join_thread)
	{
		auto&& lock = pause();

		running = false;

		if (join_thread)
		{
			event_thread.join();
		}
	}

	void EventHandler::run()
	{
		SDL_Event event;

		while (SDL_WaitEvent(&event)) // SDL_PollEvent
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					break;
				case SDL_QUIT:
					app_instance.stop();

					return;
			}
		}
	}
}