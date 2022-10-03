#pragma once

#include <engine/types.hpp>
#include <entt/signal/dispatcher.hpp>

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
		public:
			InputDevice() = default;
			InputDevice(State&& state) noexcept : state(std::move(state)) {}
			InputDevice(const State& state) : state(state) {}

			virtual ~InputDevice() {}

			virtual void peek(State& state) const = 0;

			// Override with your own implementation if needed; optional otherwise.
			virtual bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr) { return false; }

			inline const State& get_state() const { return state; }

			inline const State& poll(entt::dispatcher& event_handler) // engine::EventHandler&
			{
				peek(this->state);

				event_handler.enqueue(this->state);

				return this->state;
			}
		protected:
			State state;
	};
}