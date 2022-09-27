#include "gamepad.hpp"
#include "events.hpp"

#include <sdl2/SDL_joystick.h>

#include <optional>

// Debugging related:
#include <cmath>
#include <math/math.hpp>
#include <util/log.hpp>

namespace app::input
{
	Gamepad::Gamepad(DeviceIndex device_index, const DeadZone& deadzone, bool open_immediately)
		: device_index(device_index), deadzone(deadzone), handle(nullptr)
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
		if (is_open())
		{
			//SDL_JoystickGetDeviceProduct(device_index);
			return SDL_JoystickName(handle);
		}

		return {};
	}

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

				std::optional<GamepadAnalogInput> analog = std::nullopt;
				GamepadState::Vector direction = {};

				switch (e.jaxis.axis)
				{
					case 0: // Left stick, X-axis.
						next_state.left_analog.x = deadzone.left_analog.get_x(value);

						analog = GamepadAnalogInput::Left;
						direction = next_state.left_analog;
						
						break;

					case 1: // Left stick, Y-axis.
						next_state.left_analog.y = -deadzone.left_analog.get_y(value);

						analog = GamepadAnalogInput::Left;
						direction = next_state.left_analog;

						break;
					
					case 2: // Right stick, X-axis.
						next_state.right_analog.x = deadzone.right_analog.get_x(value);

						analog = GamepadAnalogInput::Right;
						direction = next_state.right_analog;

						break;

					case 3: // Right stick, Y-axis.
						next_state.right_analog.y = -deadzone.right_analog.get_y(value);

						analog = GamepadAnalogInput::Right;
						direction = next_state.right_analog;

						break;

					case 4: // Left trigger.
						next_state.triggers.x = deadzone.triggers.get_x(value);

						analog = GamepadAnalogInput::Triggers;
						direction = next_state.triggers;

						break;

					case 5: // Right trigger.
						next_state.triggers.y = deadzone.triggers.get_y(value);

						analog = GamepadAnalogInput::Triggers;
						direction = next_state.triggers;

						break;
				}

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

						if (opt_event_handler)
						{
							opt_event_handler->enqueue<OnGamepadAnalogInput>
							(
								device_id,
								next_state,
								GamepadAnalogInput::DPad,
								next_state.dpad_direction()
							);
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

	bool Gamepad::process_button_event(const SDL_JoyButtonEvent& e, entt::dispatcher* opt_event_handler)
	{
		assert(e.button < GamepadState::MAX_BUTTONS);

		const auto device_id = static_cast<GamepadDeviceIndex>(e.which);
		const auto button = static_cast<GamepadButtonID>(e.button);

		next_state.set_button(button, (e.state == SDL_PRESSED)); // (e.state != SDL_RELEASED)

		if (opt_event_handler)
		{
			switch (e.type)
			{
				case SDL_JOYBUTTONDOWN:
					opt_event_handler->enqueue<OnGamepadButtonDown>(device_id, next_state, button);

					break;

				case SDL_JOYBUTTONUP:
					opt_event_handler->enqueue<OnGamepadButtonUp>(device_id, next_state, button);

					break;
			}
		}

		return true;
	}
}