#pragma once

#include "types.hpp"
#include "action.hpp"

#include <utility>
#include <functional>

namespace engine
{
	class Service
	{
		protected:
			EventHandler event_handler;
		public:
			Service() = default;
			Service(Service&&) = default;

			inline Service(EventHandler&& event_handler)
				: event_handler(std::move(event_handler)) {}

			inline EventHandler& get_event_handler() { return event_handler; }

			template <typename EventType, auto fn, typename obj_type>
			inline void register_event(obj_type& obj)
			{
				event_handler.sink<EventType>().connect<fn>(obj);
			}

			template <typename EventType, auto fn, typename obj_type>
			inline void unregister_event(obj_type& obj)
			{
				event_handler.sink<EventType>().disconnect<fn>(obj);
			}

			template <typename EventType, auto fn>
			inline void register_event()
			{
				register_event<EventType, fn>(*this);
			}

			template <typename EventType, auto fn>
			inline void unregister_event()
			{
				unregister_event<EventType, fn>(*this);
			}

			template <typename EventType, typename... Args>
			inline void queue_event(Args&&... args)
			{
				event_handler.enqueue<EventType>(std::forward<Args>(args)...);
			}

			template <typename EventType>
			inline void queue_event(EventType&& event_obj)
			{
				event_handler.enqueue(std::forward<EventType>(event_obj));
			}

			template <typename EventType, typename... Args>
			inline void event(Args&&... args)
			{
				event_handler.trigger<EventType>(std::forward<Args>(args)...);
			}

			template <typename EventType>
			inline void event(EventType&& event_obj)
			{
				event_handler.trigger(std::forward<EventType>(event_obj));
			}

			// Unregisters all event triggers tied to a given object.
			template <typename obj_type>
			inline void unregister(obj_type& obj)
			{
				event_handler.disconnect(obj);
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

			inline void update()
			{
				event_handler.update();
			}
	};
}