#include "gamepad.hpp"
#include "events.hpp"
#include "gamepad_analog.hpp"
#include "gamepad_profile.hpp"
#include "gamepad_buttons.hpp"
#include "input_device_impl.hpp"

#include <math/math.hpp>

#include <sdl2/SDL_joystick.h>
#include <util/magic_enum.hpp>

#include <optional>

// Debugging related:
#include <cmath>
#include <util/log.hpp>

namespace app::input
{
	// Converts values from the range (-1.0 to 1.0) into (0.0 to 1.0).
	static auto convert_triggers(const auto& value)
	{
		//return value;
		//return math::abs((value + 1.0f) / 2.0f);
		return ((value + 1.0f) / 2.0f);
	}

	Gamepad::Gamepad
	(
		DeviceIndex device_index,
		bool event_based_button_down,
		bool event_based_button_release,
		bool continuous_analog_input,
		const DeadZone& deadzone,
		bool open_immediately
	) :
		device_index(device_index),
		event_based_button_down(event_based_button_down),
		event_based_button_release(event_based_button_release),
		continuous_analog_input(continuous_analog_input),
		deadzone(deadzone),
		handle(nullptr)
	{
		if (open_immediately)
		{
			open();
		}
	}

	Gamepad::~Gamepad()
	{
		close();
	}

	void Gamepad::apply_profile(const GamepadProfile& profile)
	{
		set_deadzone(profile.deadzone);
	}

	bool Gamepad::open(bool force)
	{
		return open(this->device_index, force);
	}

	bool Gamepad::open(DeviceIndex device_index, bool force)
	{
		if (is_open()) // (handle)
		{
			if ((device_index == this->device_index) && (!force))
			{
				return true;
			}
			else
			{
				bool closed = close();

				assert(closed);
			}
		}

		this->device_index = device_index;

		handle = SDL_JoystickOpen(static_cast<SDL_JoystickID>(device_index));

		return is_open(); // (handle);
	}

	bool Gamepad::close()
	{
		if (is_open()) // handle
		{
			SDL_JoystickClose(handle);

			handle = nullptr;

			return true;
		}

		return false;
	}

	const Gamepad::DeadZone& Gamepad::get_deadzone() const
	{
		return deadzone;
	}

	void Gamepad::set_deadzone(const DeadZone& deadzone)
	{
		this->deadzone = deadzone;
	}

	std::string Gamepad::get_device_name() const
	{
		return std::string(get_device_name_as_view());
	}

	std::string_view Gamepad::get_device_name_as_view() const
	{
		if (is_open())
		{
			//SDL_JoystickGetDeviceProduct(device_index);
			return SDL_JoystickName(handle);
		}

		return {};
	}

	const Gamepad::State& Gamepad::poll(GamepadProfile* profile, entt::dispatcher* opt_event_handler)
	{
		if (opt_event_handler)
		{
			const auto& prev_state = get_state();

			if (profile)
			{
				handle_analog_events(*opt_event_handler, *profile, next_state, prev_state);
				handle_hat_event_detection(*opt_event_handler, *profile, next_state);
			}

			handle_button_changes(*opt_event_handler, next_state, prev_state);
		}

		return poll(opt_event_handler);
	}

	/*
	const Gamepad::State& Gamepad::poll(entt::dispatcher* opt_event_handler)
	{
		// No additional actions required.

		return InputDevice<GamepadState>::poll(opt_event_handler);
	}
	*/

	void Gamepad::peek(State& state) const
	{
		state = next_state;
	}

	// TODO: Refactor into multiple subroutines.
	bool Gamepad::process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler)
	{
		switch (e.type)
		{
			case SDL_JOYAXISMOTION:
			{
				auto device_id = static_cast<GamepadDeviceIndex>(e.jaxis.which);
				auto value = e.jaxis.value;

				std::optional<GamepadAnalog> analog = std::nullopt;
				GamepadState::Vector direction = {};

				switch (e.jaxis.axis)
				{
					case 0: // Left stick, X-axis.
						next_state.left_analog.x = deadzone.left_analog.get_x(value);

						analog = GamepadAnalog::Left;
						direction = next_state.left_analog;
						
						break;

					case 1: // Left stick, Y-axis.
						next_state.left_analog.y = -deadzone.left_analog.get_y(value);

						analog = GamepadAnalog::Left;
						direction = next_state.left_analog;

						break;
					
					case 2: // Right stick, X-axis.
						next_state.right_analog.x = deadzone.right_analog.get_x(value);

						analog = GamepadAnalog::Right;
						direction = next_state.right_analog;

						break;

					case 3: // Right stick, Y-axis.
						next_state.right_analog.y = -deadzone.right_analog.get_y(value);

						analog = GamepadAnalog::Right;
						direction = next_state.right_analog;

						break;

					case 4: // Left trigger.
						next_state.triggers.x = convert_triggers(deadzone.triggers.get_x(value));

						analog = GamepadAnalog::Triggers;
						direction = next_state.triggers;

						break;

					case 5: // Right trigger.
						next_state.triggers.y = convert_triggers(deadzone.triggers.get_y(value));

						analog = GamepadAnalog::Triggers;
						direction = next_state.triggers;

						break;
				}

				if (!continuous_analog_input)
				{
					if (opt_event_handler && analog.has_value())
					{
						opt_event_handler->enqueue<OnGamepadAnalogInput>
						(
							device_id,
							next_state,
							*analog,
							direction
						);
					}
				}

				return true;
			}
			
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				return process_button_event(e.jbutton, opt_event_handler);

			case SDL_JOYHATMOTION:
			{
				switch (e.jhat.hat)
				{
					case 0: // D-Pad
					{
						auto device_id = static_cast<GamepadDeviceIndex>(e.jhat.which);

						next_state.update_dpad(e.jhat.value);

						if (!continuous_analog_input)
						{
							if (opt_event_handler)
							{
								opt_event_handler->enqueue<OnGamepadAnalogInput>
								(
									device_id,
									next_state,
									GamepadAnalog::DPad,
									next_state.dpad_direction()
								);
							}
						}

						return true;
					}

					default:
						// All other hat inputs are currently unsupported.

						break;
				}

				break;
			}

			case SDL_JOYBALLMOTION:
				// Unsupported event type.

				break;
		}

		return false;
	}

	int Gamepad::handle_button_changes(entt::dispatcher& event_handler, const State& state, const State& prev_state) const
	{
		const auto device_id = this->device_index; // const auto&

		int buttons_changed = 0;

		// Button-released detection:
		if (!event_based_button_release)
		{
			buttons_changed += state.on_button_change(prev_state, [&event_handler, device_id, &state](GamepadButtonBits button, bool value)
			{
				if (!value)
				{
					event_handler.enqueue<OnGamepadButtonUp>(device_id, state, static_cast<GamepadButtonID>(button));
				}
			});
		}

		// Continuous button-held events:
		if (!event_based_button_down)
		{
			buttons_changed += state.on_button_active([&event_handler, device_id, &state](GamepadButtonBits button)
			{
				event_handler.enqueue<OnGamepadButtonDown>(device_id, state, static_cast<GamepadButtonID>(button));
			});
		}

		return buttons_changed;
	}

	void Gamepad::handle_analog_events(entt::dispatcher& event_handler, const GamepadProfile& profile, const State& state, const State& prev_state) const
	{
		if (!continuous_analog_input)
		{
			return;
		}

		const auto device_id = this->device_index;

		auto handle_analog = [device_id, &event_handler, &profile, &state, &prev_state](GamepadAnalog analog)
		{
			const auto value = state.get_analog(analog);

			if (!value)
			{
				return;
			}

			auto should_generate_event = [&]() -> bool
			{
				if (const auto prev_value = prev_state.get_analog(analog))
				{
					//if (*value != *prev_value)
					if ((value->x != prev_value->x) || (value->y != prev_value->y))
					{
						return true;
					}
				}

				if (const auto* analog_range = profile.deadzone.get_analog(analog))
				{
					return analog_range->beyond_threshold(*value);
				}

				// Non-zero value check (generally unreliable):
				return
				(
					static_cast<bool>(value->x)
					||
					static_cast<bool>(value->y)
				);
			};

			if (!should_generate_event())
			{
				return;
			}

			event_handler.enqueue<OnGamepadAnalogInput>
			(
				device_id,
				state,
				analog,
				*value
			);
		};

		magic_enum::enum_for_each<GamepadAnalog>([&handle_analog](auto analog)
		{
			handle_analog(analog);
		});
	}

	bool Gamepad::process_button_event(const SDL_JoyButtonEvent& e, entt::dispatcher* opt_event_handler)
	{
		assert(e.button < GamepadState::MAX_BUTTONS);

		const auto device_id = static_cast<GamepadDeviceIndex>(e.which);

		// TODO: Look into converting `GamepadButtonEvent` to use `GamepadButtonBits` instead.
		const auto button_id = static_cast<GamepadButtonID>(e.button);

		next_state.set_button(button_id, (e.state == SDL_PRESSED)); // (e.state != SDL_RELEASED)

		if (opt_event_handler)
		{
			switch (e.type)
			{
				case SDL_JOYBUTTONDOWN:
					// If we're opting to use continuous button-down events instead,
					// we don't need/want to generate an event here:
					if (event_based_button_down)
					{
						opt_event_handler->enqueue<OnGamepadButtonDown>(device_id, next_state, button_id);
					}

					break; // return true;

				case SDL_JOYBUTTONUP:
					// If we're opting to use polling-based button-release events,
					// don't worry about generating an event here:
					if (event_based_button_release)
					{
						opt_event_handler->enqueue<OnGamepadButtonUp>(device_id, next_state, button_id);
					}

					break; // return true;
			}
		}

		return true; // false;
	}

	// NOTE: This method is called automatically via the profile-enabled `poll` method.
	// If this input device is used via a virtual call to `poll`, this routine will not execute as expected.
	void Gamepad::handle_hat_event_detection(entt::dispatcher& event_handler, GamepadProfile& device_profile, State& state) const
	{
		handle_hat_event_detection_impl<GamepadAnalog>
		(
			device_profile, state,

			// get_button_state:
			[](const auto& state, const auto& button)
			{
				return state.get_button(button);
			},

			[this, &event_handler](const auto& state, const auto& analog, const auto& input_direction)
			{
				const auto& device_id = this->device_index;

				event_handler.enqueue<OnGamepadAnalogInput>
				(
					device_id,
					state,
					analog,
					input_direction
				);
			}
		);
	}
}