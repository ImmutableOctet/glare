#pragma once

#include <engine/types.hpp>

#include <entt/signal/dispatcher.hpp>
//#include <entt/signal/fwd.hpp>

#include <utility>

#include <sdl2/SDL_events.h>

//union SDL_Event;

/*
namespace entt
{
	class dispatcher;
}
*/

namespace app::input
{
	template <typename T>
	struct InputDevice
	{
		public:
			using State = T;
			
			InputDevice() = default;
			InputDevice(State&& state) noexcept : state(std::move(state)) {}
			InputDevice(const State& state) : state(state) {}

			virtual ~InputDevice() {}

			//InputDevice& operator=(const InputDevice&) = default;
			//InputDevice& operator=(InputDevice&&) noexcept = default;

			/*
				Called any time the internal state is queried. (This includes `poll`)
				`state` indicates an output object to copy state contents to.
				
				NOTE:
					`state` may be the same object as `this->state`,
					and in the case of `poll`, it is defined as such.
			*/
			virtual void peek(State& state) const = 0;

			// Intended to perform any reset operations after polling.
			// (e.g. reset or update relative motion values)
			// This is called during `poll`, after use of `peek`.
			virtual void flush()
			{
				// Default implementation; do nothing.
			}

			// Override with your own implementation if needed; optional otherwise.
			virtual bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr) { return false; }

			/*
				Retrieves a temporary reference to the last posted input state.
				
				NOTE:
					This may differ from an input state advertised by a device using an event-handler.
				
					Likewise, this represents the last posted snapshot of the input state,
					and does not represent an input state still being processed.
			*/
			inline const State& get_state() const { return state; }

			// Polls for the next input state, updating the 'last-posted-snapshot' accordingly.
			// The return value is the same object returned by `get_state`.
			// If overriding this method, you must call back to this implementation.
			virtual const State& poll(entt::dispatcher* opt_event_handler=nullptr) // engine::EventHandler*
			{
				// Request the next input state.
				peek(this->state);

				if (opt_event_handler)
				{
					submit_state(*opt_event_handler, this->state);
				}

				// Perform any flush operations required by the device.
				flush();

				// Return the snapshot we took.
				return this->state;
			}

		protected:
			virtual void submit_state(entt::dispatcher& event_handler, const State& state) // const
			{
				// Post the input state to the event-handler.
				event_handler.enqueue(state);
			}
		private:
			/*
				A snapshot of this device's state at a given point in time.
				This is updated periodically via the `poll` command.
				
				NOTE:
					This object should not be modified outside of `peek`.

					Any temporary storage required to process messages, handle events, etc.
					should be managed separately by the device implementation.
			*/
			State state;
	};
}