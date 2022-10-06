#pragma once

#include "types.hpp"
#include "input_device.hpp"
#include "keyboard_state.hpp"

namespace app::input
{
	class Keyboard : public InputDevice<KeyboardState>
	{
		public:
			Keyboard();

			virtual void peek(State& state) const override;
			//bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr) override { return false; }

			std::string get_device_name() const;
			std::string_view peek_device_name() const;
	};
}