#include "input_event.hpp"

#include <util/variant.hpp>
#include <app/input/input_states.hpp>
#include <app/input/devices.hpp>

namespace engine
{
	const app::input::Mouse* InputEvent::get_mouse() const
	{
		return util::get_if_wrapped<const app::input::Mouse>(source);
	}

	const app::input::Keyboard* InputEvent::get_keyboard() const
	{
		return util::get_if_wrapped<const app::input::Keyboard>(source);
	}

	const app::input::Gamepad* InputEvent::get_gamepad() const
	{
		return util::get_if_wrapped<const app::input::Gamepad>(source);
	}

	const app::input::MouseState* InputEvent::get_mouse_state() const
	{
		auto* mouse = get_mouse();

		if (!mouse)
		{
			return {};
		}

		return &mouse->get_state();
	}

	const app::input::KeyboardState* InputEvent::get_keyboard_state() const
	{
		auto* keyboard = get_keyboard();

		if (!keyboard)
		{
			return {};
		}

		return &keyboard->get_state();
	}

	const app::input::GamepadState* InputEvent::get_gamepad_state() const
	{
		auto* gamepad = get_gamepad();

		if (!gamepad)
		{
			return {};
		}

		return &gamepad->get_state();
	}

	bool InputEvent::is_mouse_event() const
	{
		//return get_mouse();
		return source_is<app::input::Mouse>();
	}

	bool InputEvent::is_keyboard_event() const
	{
		//return get_keyboard();
		return source_is<app::input::Keyboard>();
	}

	bool InputEvent::is_gamepad_event() const
	{
		//return get_gamepad();
		return source_is<app::input::Gamepad>();
	}

	bool InputEvent::is_monostate_event() const
	{
		return (source_index() == UnknownIndex); // util::variant_index<InputSource, std::monostate>()
	}

	// Indicates the internal variant-index of `source`.
	std::size_t InputEvent::source_index() const
	{
		return source.index();
	}
}