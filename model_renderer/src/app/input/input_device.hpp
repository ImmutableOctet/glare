#pragma once

#include <engine/types.hpp>

namespace app
{
	namespace input
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

				inline State poll(engine::EventHandler& event_handler)
				{
					this->state = peek();

					event_handler.enqueue(this->state);

					return this->state;
				}
		};
	}
}