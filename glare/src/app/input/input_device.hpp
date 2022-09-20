#pragma once

#include <engine/types.hpp>
#include <entt/signal/dispatcher.hpp>

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
		protected:
			State state;
		public:
			virtual State peek() = 0;

			inline State get_state() const { return state; }

			inline State poll(entt::dispatcher& event_handler) // engine::EventHandler&
			{
				this->state = peek();

				event_handler.enqueue(this->state);

				return this->state;
			}
	};
}