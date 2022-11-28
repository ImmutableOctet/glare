#include "service.hpp"
#include "input/raw_input_events.hpp"

#include "meta/meta.hpp"

#include <app/input/mouse_state.hpp>
#include <app/input/keyboard_state.hpp>
//#include <app/input/gamepad_state.hpp>

#include <algorithm>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	Service::Service(bool register_input_events, bool register_timed_event_wrapper)
		: active_event_handler(&standard_event_handler)
	{
		if (register_input_events)
		{
			// Only polled device states for now.
			register_event<app::input::MouseState, &Service::on_mouse_input>(*this);
			register_event<app::input::KeyboardState, &Service::on_keyboard_input>(*this);

			// TODO: Implement exact device events, rather than exclusively polled states.
		}

		if (register_timed_event_wrapper)
		{
			register_event<TimedEvent, &Service::enqueue_timed_event_wrapper>(*this);
		}
	}

	EventHandler* Service::swap_event_handlers()
	{
		if (active_event_handler == &standard_event_handler)
		{
			return use_forwarding_events();
		}
		else if (active_event_handler == &forwarding_event_handler)
		{
			return use_standard_events();
		}

		return active_event_handler;
	}

	EventHandler* Service::use_standard_events()
	{
		active_event_handler = &standard_event_handler;

		return active_event_handler;
	}

	EventHandler* Service::use_forwarding_events()
	{
		active_event_handler = &forwarding_event_handler;

		return active_event_handler;
	}

	// TODO: Implement thread-safe locking/synchronization event-handler interface.
	void Service::update(float delta)
	{
		// Handle standard events:
		use_forwarding_events();
		standard_event_handler.update();

		// TODO: Implement thread/async-driven timed-event detection and queueing.
		update_timed_events();

		// Handle forwarding events:
		use_standard_events();
		forwarding_event_handler.update();

		// Trigger the standard update event for this service.
		this->event<OnServiceUpdate>(this, delta);
	}

	void Service::render(app::Graphics& gfx)
	{
		this->event<OnServiceRender>(this, &gfx);
	}

	EventHandler& Service::get_active_event_handler()
	{
		assert(active_event_handler);

		return *active_event_handler;
	}

	EventHandler& Service::get_standard_event_handler()
	{
		return standard_event_handler;
	}

	EventHandler& Service::get_forwarding_event_handler()
	{
		return forwarding_event_handler;
	}

	// Input re-submission callbacks (see class declaration(s) for details):
	void Service::on_mouse_input(const app::input::MouseState& mouse)
	{
		event<OnMouseState>(this, &mouse);
	}

	void Service::on_keyboard_input(const app::input::KeyboardState& keyboard)
	{
		event<OnKeyboardState>(this, &keyboard);
	}

	void Service::enqueue_timed_event_wrapper(const TimedEvent& timed_event)
	{
		enqueue_timed_event(TimedEvent(timed_event));
	}

	void Service::enqueue_timed_event(TimedEvent&& timed_event)
	{
		pending_timed_events.emplace_back(std::forward<TimedEvent>(timed_event));
	}

	// TODO: Look into whether sorting the `pending_timed_events` collection would improve performance.
	void Service::update_timed_events()
	{
		pending_timed_events.erase
		(
			std::remove_if
			(
				pending_timed_events.begin(),
				pending_timed_events.end(),

				[this](TimedEvent& timed_event)
				{
					using namespace entt::literals;

					if (timed_event.completed())
					{
						auto type = timed_event.type();

						// Timed events must represent a reflected type, due to type-erasure.
						assert(type);

						if (!type)
						{
							print_error("Unable to resolve underlying timed event type.");

							return true;
						}

						auto trigger_fn = type.func("trigger_event_from_meta_any"_hs);

						assert(trigger_fn);

						if (!trigger_fn)
						{
							print_error("Unable to resolve event-trigger function for underlying timed event type.");

							return true;
						}

						trigger_fn.invoke
						(
							{},
							entt::forward_as_meta(*this),
							entt::forward_as_meta(std::move(timed_event.event_instance))
						);

						return true;
					}

					return false;
				}
			),

			pending_timed_events.end()
		);
	}
}