#pragma once

#include "types.hpp"
#include "action.hpp"

#include "service_events.hpp"
#include "timed_event.hpp"
#include "timer.hpp"

#include "meta/types.hpp"

#include <util/small_vector.hpp>

#include <utility>
#include <functional>
#include <type_traits>
#include <optional>
//#include <vector>

// Debugging related:
//#include <util/log.hpp>

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
	class ResourceManager;

	class Service
	{
		public:
			Service(bool register_input_events=true, bool register_timed_event_wrapper=false);
			Service(Service&&) noexcept = default;

			virtual ~Service() {};

			virtual Registry& get_registry() = 0;
			virtual const Registry& get_registry() const = 0;

			virtual ResourceManager& get_resource_manager() = 0;
			virtual const ResourceManager& get_resource_manager() const = 0;

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
				if constexpr (std::is_same_v<EventType, std::decay_t<TimedEvent>>)
				{
					enqueue_timed_event(TimedEvent { std::forward<Args>(args)... });
				}
				else
				{
					active_event_handler->enqueue<EventType>(EventType{ std::forward<Args>(args)... });
				}
			}

			template <typename EventType>
			inline void queue_event(EventType&& event_obj)
			{
				active_event_handler->enqueue(std::forward<EventType>(event_obj));
			}

			template <>
			inline void queue_event<TimedEvent>(TimedEvent&& event_obj)
			{
				enqueue_timed_event(std::move(event_obj));
			}

			template <typename EventType, typename... Args>
			inline void event(Args&&... args)
			{
				if constexpr (std::is_same_v<EventType, std::decay_t<TimedEvent>>)
				{
					queue_event<EventType, Args...>(std::forward<Args>(args)...);
				}
				else
				{
					active_event_handler->trigger<EventType>(EventType{ std::forward<Args>(args)... });
				}
			}

			template <typename EventType>
			inline void event(EventType&& event_obj)
			{
				active_event_handler->trigger(std::forward<EventType>(event_obj));
			}

			template <>
			inline void event<TimedEvent>(TimedEvent&& event_obj)
			{
				queue_event<TimedEvent>(std::forward<TimedEvent>(event_obj));
			}

			template <>
			inline void event<MetaAny>(MetaAny&& event_obj)
			{
				trigger_opaque_event(std::move(event_obj));
			}

			template <typename EventType, typename... Args>
			inline void timed_event(Timer timer, Args&&... args)
			{
				queue_event<TimedEvent>(timer, EventType { std::forward<Args>(args)... });
			}

			template <typename EventType>
			inline void timed_event(Timer timer, EventType&& event_obj)
			{
				queue_event<TimedEvent>(TimedEvent(timer, std::move(event_obj)));
			}


			template <typename EventType, typename... Args>
			inline void timed_event(Timer::Duration timer_duration, Args&&... args)
			{
				timed_event<EventType>(Timer(timer_duration), std::forward<Args>(args)...);
			}

			template <typename EventType, typename... Args>
			inline void timed_event(std::optional<Timer> timer, Args&&... args)
			{
				if (!timer)
				{
					queue_event<EventType>(std::forward<Args>(args)...);

					return;
				}

				timed_event<EventType>(std::move(*timer), std::forward<Args>(args)...);
			}

			template <typename EventType, typename... Args>
			inline void timed_event(std::optional<Timer::Duration> timer_duration, Args&&... args)
			{
				if (!timer_duration)
				{
					queue_event<EventType>(std::forward<Args>(args)...);

					return;
				}

				timed_event<EventType>(Timer(*timer_duration), std::forward<Args>(args)...);
			}

			template <typename TimeType>
			inline void timed_event(std::optional<TimeType> timer_duration, MetaAny&& event_instance)
			{
				if (timer_duration)
				{
					queue_event<TimedEvent>
					(
						TimedEvent
						{
							std::move(*timer_duration),
							std::move(event_instance)
						}
					);
				}
				else
				{
					trigger_opaque_event(std::move(event_instance));
				}
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

			void update(float delta=1.0f);
			void render(app::Graphics& gfx);

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

			// Interop functionality with standard event-handler systems.
			void enqueue_timed_event_wrapper(const TimedEvent& timed_event);

			// Regular rvalue input from directly enqueued timed-event objects.
			void enqueue_timed_event(TimedEvent&& timed_event);

			// Enumerates timed events, triggering underlying event types when timers complete.
			void update_timed_events();

			// Triggers the the event method for the underlying object in `event_instance`.
			// The return-value of this method indicates success.
			bool trigger_opaque_event(MetaAny&& event_instance);

			// A pointer to the active event handler object.
			// (One of the two found below)
			EventHandler* active_event_handler;

			// Standard event handler; used under most conditions.
			EventHandler standard_event_handler;

			// Forwarding event handler; used while processing events.
			EventHandler forwarding_event_handler;

			util::small_vector<TimedEvent, 8> pending_timed_events;
	};
}