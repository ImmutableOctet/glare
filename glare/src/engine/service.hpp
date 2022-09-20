#pragma once

#include "types.hpp"
#include "action.hpp"

#include "service_events.hpp"

#include <utility>
#include <functional>

namespace app
{
	namespace input
	{
		struct MouseState;
		struct KeyboardState;
	}

	namespace graphics
	{
		struct Graphics;
	}
}

namespace engine
{
	class Service
	{
		public:
			Service(bool register_input_events=true);
			Service(Service&&) = default;

			virtual ~Service() {};

			virtual Registry& get_registry() = 0;
			virtual const Registry& get_registry() const = 0;

			template <typename EventType, auto fn, typename obj_type>
			inline void register_event(obj_type& obj)
			{
				standard_event_handler.sink<EventType>().connect<fn>(obj);
				forwarding_event_handler.sink<EventType>().connect<fn>(obj);
			}

			template <typename EventType, auto fn, typename obj_type>
			inline void unregister_event(obj_type& obj)
			{
				standard_event_handler.sink<EventType>().disconnect<fn>(obj);
				forwarding_event_handler.sink<EventType>().disconnect<fn>(obj);
			}

			template <typename EventType, typename... Args>
			inline void queue_event(Args&&... args)
			{
				active_event_handler->enqueue<EventType>(EventType { std::forward<Args>(args)... });
			}

			template <typename EventType>
			inline void queue_event(EventType&& event_obj)
			{
				active_event_handler->enqueue(std::forward<EventType>(event_obj));
			}

			template <typename EventType, typename... Args>
			inline void event(Args&&... args)
			{
				active_event_handler->trigger<EventType>(EventType { std::forward<Args>(args)... });
			}

			template <typename EventType>
			inline void event(EventType&& event_obj)
			{
				active_event_handler->trigger(std::forward<EventType>(event_obj));
			}

			// Unregisters all event triggers tied to a given object.
			template <typename obj_type>
			inline void unregister(obj_type& obj)
			{
				standard_event_handler.disconnect(obj);
				forwarding_event_handler.disconnect(obj);
			}

			template <typename EventType, auto fn>
			inline void register_free_function()
			{
				standard_event_handler.sink<EventType>().connect<fn>();
				forwarding_event_handler.sink<EventType>().connect<fn>();
			}

			template <typename EventType, auto fn>
			inline void unregister_free_function()
			{
				standard_event_handler.sink<EventType>().disconnect<fn>();
				forwarding_event_handler.sink<EventType>().disconnect<fn>();
			}

			// Queues `fn` to be executed with `args` on the next call to `update`.
			// Deferred calls (`Actions`) are executed in-order for the function-type specified.
			// This means that lambdas can be used to uniquely queue a particular action in FIFO order.
			// For a good example of this, see `World::set_parent`.
			// NOTE: `fn` does not currently support lambdas with captures. Regular lambdas should work fine, though.
			template <typename fn_t, typename... arguments>
			inline void defer(fn_t&& f, arguments&&... args)
			{
				queue_event(make_action(std::forward<fn_t>(f), std::forward<arguments>(args)...));

				// Debugging related:
				//make_action(std::forward<fn_t>(f), std::forward<arguments>(args)...);
			}

			inline void update(float delta=1.0f)
			{
				// Handle standard events:
				use_forwarding_events();
				standard_event_handler.update();

				// Handle forwarding events:
				use_standard_events();
				forwarding_event_handler.update();

				// Trigger the standard update event for this service.
				this->event<OnServiceUpdate>(this, delta);
			}

			inline void render(app::Graphics& gfx)
			{
				this->event<OnServiceRender>(this, &gfx);
			}

			// NOTE: Registering to this event handler is considered unsafe due to there
			// being 'standard' and 'forwarding' event handlers internally. Use this method with caution.
			EventHandler& get_active_event_handler();

			// See `get_active_event_handler` for notes on using service-owned event handlers directly.
			EventHandler& get_standard_event_handler();

			// See `get_active_event_handler` for notes on using service-owned event handlers directly.
			EventHandler& get_forwarding_event_handler();

		protected:
			EventHandler* swap_event_handlers();
			EventHandler* use_standard_events();
			EventHandler* use_forwarding_events();

			// Input re-submission callbacks. -- These routines produce additional
			// events indicating that this service accepted an input.
			// For more details, see `input_events`.
			void on_mouse_input(const app::input::MouseState& mouse);
			void on_keyboard_input(const app::input::KeyboardState& keyboard);

			// A pointer to the active event handler object.
			// (One of the two found below)
			EventHandler* active_event_handler;

			// Standard event handler; used under most conditions.
			EventHandler standard_event_handler;

			// Forwarding event handler; used while processing events.
			EventHandler forwarding_event_handler;
	};
}