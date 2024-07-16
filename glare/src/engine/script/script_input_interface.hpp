#pragma once

#include <engine/types.hpp>

#include <engine/input/types.hpp>
#include <engine/input/events.hpp>

namespace engine
{
	class ScriptInputInterface
	{
		public:
			decltype(auto) until_input_changed(this auto&& self, PlayerIndex player_index)
			{
				return self.template until<OnInput>
				(
					[player_index](const OnInput& player_input)
					{
						return (player_input.player_index() == player_index);
					}
				);
			}

			decltype(auto) until_input_changed(this auto&& self)
			{
				return self.until_input_changed(self.get_target_player_index());
			}

			decltype(auto) until_any_player_input_changed(this auto&& self)
			{
				return self.template until<OnInput>();
			}

			decltype(auto) until_input_state(this auto&& self, PlayerIndex player_index)
			{
				return self.template until<OnInputState>
				(
					[player_index](const OnInputState& player_input_state)
					{
						return (player_input_state.player_index() == player_index);
					}
				);
			}

			decltype(auto) until_input_state(this auto&& self)
			{
				return self.until_input_state(self.get_target_player_index());
			}

			decltype(auto) until_input(this auto&& self, PlayerIndex player_index)
			{
				return self.until_input_state(player_index);
			}

			decltype(auto) until_input(this auto&& self)
			{
				return self.until_input(self.get_target_player_index());
			}

			decltype(auto) until_any_player_input_state(this auto&& self)
			{
				return self.template until<OnInputState>();
			}

			decltype(auto) until_button_pressed(this auto&& self, PlayerIndex player_index, Button button)
			{
				return self.template until<OnInput>
				(
					[player_index, button](const OnInput& player_input)
					{
						return ((player_input.player_index() == player_index) && (player_input.get_state().pressed.get_button(button)));
					}
				);
			}

			decltype(auto) until_button_pressed(this auto&& self, PlayerIndex player_index)
			{
				return self.template until<OnInput>
				(
					[player_index](const OnInput& player_input)
					{
						return ((player_input.player_index() == player_index) && (player_input.get_state().pressed.any()));
					}
				);
			}

			decltype(auto) until_button_pressed(this auto&& self, Button button)
			{
				return self.until_button_pressed(self.get_target_player_index(), button);
			}

			decltype(auto) until_button_pressed(this auto&& self)
			{
				return self.until_button_pressed(self.get_target_player_index());
			}

			decltype(auto) until_any_button_pressed(this auto&& self)
			{
				return self.until_button_pressed();
			}

			decltype(auto) until_any_player_pressed_button(this auto&& self, Button button)
			{
				return self.template until<OnInput>
				(
					[button](const OnInput& any_player_input)
					{
						return (any_player_input.get_state().pressed.get_button(button));
					}
				);
			}

			decltype(auto) until_any_player_pressed_button(this auto&& self)
			{
				return self.template until<OnInput>
				(
					[](const OnInput& any_player_input)
					{
						return (any_player_input.get_state().pressed.any());
					}
				);
			}

			decltype(auto) until_analog_changed(this auto&& self, PlayerIndex player_index, Analog analog)
			{
				return self.template until<OnAnalogInput>
				(
					[player_index, analog](const OnAnalogInput& analog_input)
					{
						return ((analog_input.player_index() == player_index) && (analog_input.analog == analog));
					}
				);
			}

			decltype(auto) until_analog_changed(this auto&& self, PlayerIndex player_index)
			{
				return self.template until<OnAnalogInput>
				(
					[player_index](const OnAnalogInput& analog_input)
					{
						return (analog_input.player_index() == player_index);
					}
				);
			}

			decltype(auto) until_analog_changed(this auto&& self, Analog analog)
			{
				return self.until_analog_changed(self.get_target_player_index(), analog);
			}

			decltype(auto) until_analog_changed(this auto&& self)
			{
				return self.until_analog_changed(self.get_target_player_index());
			}

			decltype(auto) until_any_analog_changed(this auto&& self)
			{
				return self.until_analog_changed();
			}

			decltype(auto) until_analog_state(this auto&& self, PlayerIndex player_index, Analog analog)
			{
				return self.template until<OnAnalogInputState>
				(
					[player_index, analog](const OnAnalogInputState& analog_input)
					{
						return ((analog_input.player_index() == player_index) && (analog_input.analog == analog));
					}
				);
			}

			decltype(auto) until_analog_state(this auto&& self, PlayerIndex player_index)
			{
				return self.template until<OnAnalogInputState>
				(
					[player_index](const OnAnalogInputState& analog_input)
					{
						return (analog_input.player_index() == player_index);
					}
				);
			}

			decltype(auto) until_analog_state(this auto&& self, Analog analog)
			{
				return self.until_analog_state(self.get_target_player_index(), analog);
			}

			decltype(auto) until_analog_state(this auto&& self)
			{
				return self.until_analog_state(self.get_target_player_index());
			}

			decltype(auto) until_any_analog_state(this auto&& self)
			{
				return self.until_analog_state();
			}

			decltype(auto) until_analog(this auto&& self, auto&&... args)
			{
				return self.until_analog_changed(std::forward<decltype(args)>(args)...);
			}

			decltype(auto) until_any_analog(this auto&& self)
			{
				return self.until_any_analog_changed();
			}
	};
}