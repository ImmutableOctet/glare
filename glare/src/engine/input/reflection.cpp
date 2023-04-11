#pragma once

#include "reflection.hpp"

#include "components/input_component.hpp"

#include "buttons.hpp"
#include "events.hpp"
#include "raw_input_events.hpp"

#include <app/input/devices.hpp>
#include <app/input/events.hpp>

#include <app/input/gamepad_deadzone.hpp>

namespace engine
{
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnButtonPressed,  ButtonEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnButtonReleased, ButtonEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnButtonDown,     ButtonEvent); // OnButtonHeld

	template <>
	void reflect<InputComponent>()
	{
		//REFLECT_SINGLE_FIELD_COMPONENT(InputComponent, input_index);

		engine_meta_type<InputComponent>()
			.data<nullptr, &InputComponent::get_input_index>("input_index"_hs)
			.ctor<decltype(InputComponent::input_index)>()
		;
	}

	template <>
	void reflect<InputEvent>()
	{
		engine_meta_type<InputEvent>()
			//.data<&InputEvent::source>("source"_hs)
			
			.data<&InputEvent::state_index>("state_index"_hs)
			.data<&InputEvent::state_index>("player_index"_hs)
			//.data<&InputEvent::state_index>("player"_hs)

			.data<&InputEvent::state>("state"_hs)

			.data<nullptr, &InputEvent::get_mouse>("mouse"_hs)
			.data<nullptr, &InputEvent::get_keyboard>("keyboard"_hs)
			.data<nullptr, &InputEvent::get_gamepad>("gamepad"_hs)
			.data<nullptr, &InputEvent::get_mouse_state>("mouse_state"_hs)
			.data<nullptr, &InputEvent::get_keyboard_state>("keyboard_state"_hs)
			.data<nullptr, &InputEvent::get_gamepad_state>("gamepad_state"_hs)

			.data<nullptr, &InputEvent::is_mouse_event>("is_mouse_event"_hs)
			.data<nullptr, &InputEvent::is_keyboard_event>("is_keyboard_event"_hs)
			.data<nullptr, &InputEvent::is_gamepad_event>("is_gamepad_event"_hs)
			.data<nullptr, &InputEvent::is_monostate_event>("is_monostate_event"_hs)
			.data<nullptr, &InputEvent::has_known_input_source>("has_known_input_source"_hs)
			.data<nullptr, &InputEvent::has_unknown_input_source>("has_unknown_input_source"_hs)
			.data<nullptr, &InputEvent::source_index>("source_index"_hs)

			/*
			.data<InputEvent::UnknownIndex>("UnknownIndex"_hs)
			.data<InputEvent::MouseIndex>("MouseIndex"_hs)
			.data<InputEvent::KeyboardIndex>("KeyboardIndex"_hs)
			.data<InputEvent::GamepadIndex>("GamepadIndex"_hs)
			.data<InputEvent::NetworkIndex>("NetworkIndex"_hs)
			*/
		;
	}

	template <>
	void reflect<OnInput>()
	{
		engine_meta_type<OnInput>()
			.data<&OnInput::previous_state>("previous_state"_hs)
		;
	}

	template <>
	void reflect<ButtonEvent>()
	{
		engine_meta_type<ButtonEvent>()
			.data<&ButtonEvent::button>("button"_hs)
			.base<InputEvent>()
		;
	}

	template <>
	void reflect<OnAnalogInput>()
	{
		engine_meta_type<OnAnalogInput>()
			.data<&OnAnalogInput::analog>("analog"_hs)
			.data<&OnAnalogInput::value>("value"_hs)
			.data<&OnAnalogInput::angle>("angle"_hs)
		;
	}

	template <>
	void reflect<OnKeyboardState>()
	{
		engine_meta_type<OnKeyboardState>()
			.data<&OnKeyboardState::service>("service"_hs)
			.data<&OnKeyboardState::keyboard_state>("keyboard_state"_hs)
		;
	}

	template <>
	void reflect<OnMouseState>()
	{
		engine_meta_type<OnMouseState>()
			.data<&OnMouseState::service>("service"_hs)
			.data<&OnMouseState::mouse_state>("mouse_state"_hs)
		;
	}

	template <>
	void reflect<app::input::MouseState>()
	{
		custom_meta_type<app::input::MouseState>("MouseState"_hs)
			.data<&app::input::MouseState::x>("x"_hs)
			.data<&app::input::MouseState::y>("y"_hs)
			.data<&app::input::MouseState::wheel_x>("wheel_x"_hs)
			.data<&app::input::MouseState::wheel_y>("wheel_y"_hs)

			.func<&app::input::MouseState::get_button>("get_button"_hs)
		;
	}

	template <>
	void reflect<app::input::GamepadState>()
	{
		custom_meta_type<app::input::GamepadState>("GamepadState"_hs)
			.data<&app::input::GamepadState::left_analog>("left_analog"_hs)
			.data<&app::input::GamepadState::right_analog>("right_analog"_hs)
			.data<&app::input::GamepadState::triggers>("triggers"_hs)
			.data<&app::input::GamepadState::buttons>("buttons"_hs)

			.data<nullptr, &app::input::GamepadState::dpad_direction>("dpad_direction"_hs)
			.data<nullptr, &app::input::GamepadState::dpad_angle>("dpad_angle"_hs)
			.data<nullptr, &app::input::GamepadState::no_directional_input>("no_directional_input"_hs)
			.data<nullptr, &app::input::GamepadState::has_directional_input>("has_directional_input"_hs)

			.func<&app::input::GamepadState::update_buttons>("update_buttons"_hs)
			.func<&app::input::GamepadState::get_dpad>("get_dpad"_hs)
			.func<&app::input::GamepadState::get_dpad_button>("get_dpad_button"_hs)
			.func<&app::input::GamepadState::get_analog>("get_analog"_hs)
			.func<&app::input::GamepadState::clear_dpad>("clear_dpad"_hs)
			.func<&app::input::GamepadState::update_dpad>("update_dpad"_hs)

			.func<static_cast<void (app::input::GamepadState::*)(app::input::GamepadButtonID, bool)>(&app::input::GamepadState::set_button)>("set_button"_hs)
			.func<static_cast<void (app::input::GamepadState::*)(app::input::GamepadState::ButtonBit, bool)>(&app::input::GamepadState::set_button)>("set_button"_hs)

			.func<static_cast<bool (app::input::GamepadState::*)(app::input::GamepadState::ButtonBit) const>(&app::input::GamepadState::get_button)>("get_button"_hs)
			.func<static_cast<bool (app::input::GamepadState::*)(app::input::GamepadButtonID) const>(&app::input::GamepadState::get_button)>("get_button"_hs)

			.data<&app::input::GamepadState::MAX_BUTTONS>("MAX_BUTTONS"_hs)
			.data<&app::input::GamepadState::DPAD_BIT_OFFSET>("DPAD_BIT_OFFSET"_hs)
		;
	}

	template <>
	void reflect<app::input::KeyboardState>()
	{
		custom_meta_type<app::input::KeyboardState>("KeyboardState"_hs)
			.data<&app::input::KeyboardState::keys>("keys"_hs)
			.data<nullptr, &app::input::KeyboardState::has_keys>("has_keys"_hs)

			.func<&app::input::KeyboardState::set_keys>("set_keys"_hs)
			.func<&app::input::KeyboardState::clear_keys>("clear_keys"_hs)
			.func<&app::input::KeyboardState::set_key>("set_key"_hs)
			.func<static_cast<bool(app::input::KeyboardState::*)(app::input::KeyboardButton) const>(&app::input::KeyboardState::get_key)>("get_key"_hs)
			.func<static_cast<bool(app::input::KeyboardState::*)(std::optional<app::input::KeyboardButton>) const>(&app::input::KeyboardState::get_key)>("get_key"_hs)
			.func<static_cast<bool(app::input::KeyboardState::*)(app::input::KeyboardState::SizeType) const>(&app::input::KeyboardState::get_key)>("get_key"_hs)
			.func<&app::input::KeyboardState::clear_key>("clear_key"_hs)

			.data<&app::input::KeyboardState::MAX_KEYS>("MAX_KEYS"_hs)
		;
	}

	template <>
	void reflect<app::input::MouseStateEvent>()
	{
		custom_meta_type<app::input::MouseStateEvent>("MouseStateEvent"_hs)
			.data<&app::input::MouseStateEvent::device_index>("device_index"_hs)
			.data<&app::input::MouseStateEvent::state>("state"_hs)
		;
	}

	template <>
	void reflect<app::input::GamepadStateEvent>()
	{
		custom_meta_type<app::input::GamepadStateEvent>("GamepadStateEvent"_hs)
			.data<&app::input::GamepadStateEvent::device_index>("device_index"_hs)
			.data<&app::input::GamepadStateEvent::state>("state"_hs)
		;
	}

	template <>
	void reflect<app::input::KeyboardStateEvent>()
	{
		custom_meta_type<app::input::KeyboardStateEvent>("KeyboardStateEvent"_hs)
			.data<&app::input::KeyboardStateEvent::device_index>("device_index"_hs)
			.data<&app::input::KeyboardStateEvent::state>("state"_hs)
		;
	}

	template <>
	void reflect<app::input::MouseButtonEvent>()
	{
		custom_meta_type<app::input::MouseButtonEvent>("MouseButtonEvent"_hs)
			.base<app::input::MouseStateEvent>()
			
			.data<&app::input::MouseButtonEvent::button>("button"_hs)
		;
	}

	template <>
	void reflect<app::input::GamepadButtonEvent>()
	{
		custom_meta_type<app::input::GamepadButtonEvent>("GamepadButtonEvent"_hs)
			.base<app::input::GamepadStateEvent>()
			
			.data<&app::input::GamepadButtonEvent::button>("button"_hs)
		;
	}

	template <>
	void reflect<app::input::KeyboardButtonEvent>()
	{
		custom_meta_type<app::input::KeyboardButtonEvent>("KeyboardButtonEvent"_hs)
			.base<app::input::KeyboardStateEvent>()
			
			.data<&app::input::KeyboardButtonEvent::button>("button"_hs)
		;
	}

	template <>
	void reflect<app::input::MouseAnalogEvent>()
	{
		custom_meta_type<app::input::MouseAnalogEvent>("MouseAnalogEvent"_hs)
			.base<app::input::MouseStateEvent>()
			
			.data<&app::input::MouseAnalogEvent::analog>("analog"_hs)
		;
	}

	template <>
	void reflect<app::input::OnMouseButtonDown>()
	{
		custom_meta_type<app::input::OnMouseButtonDown>("OnMouseButtonDown"_hs)
			.base<app::input::MouseButtonEvent>()
		;
	}

	template <>
	void reflect<app::input::OnMouseButtonUp>()
	{
		custom_meta_type<app::input::OnMouseButtonUp>("OnMouseButtonUp"_hs)
			.base<app::input::MouseButtonEvent>()
		;
	}

	template <>
	void reflect<app::input::OnMouseMove>()
	{
		custom_meta_type<app::input::OnMouseMove>("OnMouseMove"_hs)
			.base<app::input::MouseAnalogEvent>()
			
			.data<&app::input::OnMouseMove::x>("x"_hs)
			.data<&app::input::OnMouseMove::y>("y"_hs)
		;
	}

	template <>
	void reflect<app::input::OnMouseScroll>()
	{
		custom_meta_type<app::input::OnMouseScroll>("OnMouseScroll"_hs)
			.base<app::input::MouseAnalogEvent>()
			
			.data<&app::input::OnMouseScroll::wheel_x>("wheel_x"_hs)
			.data<&app::input::OnMouseScroll::wheel_y>("wheel_y"_hs)
		;
	}

	template <>
	void reflect<app::input::OnMouseVirtualAnalogInput>()
	{
		custom_meta_type<app::input::OnMouseVirtualAnalogInput>("OnMouseVirtualAnalogInput"_hs)
			.base<app::input::MouseAnalogEvent>()

			.data<&app::input::OnMouseVirtualAnalogInput::value>("value"_hs)
		;
	}

	template <>
	void reflect<app::input::OnGamepadConnected>()
	{
		custom_meta_type<app::input::OnGamepadConnected>("OnGamepadConnected"_hs)
			.data<&app::input::OnGamepadConnected::device_index>("device_index"_hs)
		;
	}

	template <>
	void reflect<app::input::OnGamepadDisconnected>()
	{
		custom_meta_type<app::input::OnGamepadDisconnected>("OnGamepadDisconnected"_hs)
			.data<&app::input::OnGamepadDisconnected::device_index>("device_index"_hs)
		;
	}

	template <>
	void reflect<app::input::OnGamepadButtonDown>()
	{
		custom_meta_type<app::input::OnGamepadButtonDown>("OnGamepadButtonDown"_hs)
			.base<app::input::GamepadButtonEvent>()
		;
	}

	template <>
	void reflect<app::input::OnGamepadButtonUp>()
	{
		custom_meta_type<app::input::OnGamepadButtonUp>("OnGamepadButtonUp"_hs)
			.base<app::input::GamepadButtonEvent>()
		;
	}

	template <>
	void reflect<app::input::OnGamepadAnalogInput>()
	{
		custom_meta_type<app::input::OnGamepadAnalogInput>("OnGamepadAnalogInput"_hs)
			.base<app::input::GamepadStateEvent>()

			.data<&app::input::OnGamepadAnalogInput::analog>("analog"_hs)
			.data<&app::input::OnGamepadAnalogInput::value>("value"_hs)
			
			.data<nullptr, &app::input::OnGamepadAnalogInput::angle>("angle"_hs)
		;
	}

	template <>
	void reflect<app::input::OnKeyboardButtonDown>()
	{
		custom_meta_type<app::input::OnKeyboardButtonDown>("OnKeyboardButtonDown"_hs)
			.base<app::input::KeyboardButtonEvent>()
		;
	}
	
	template <>
	void reflect<app::input::OnKeyboardButtonUp>()
	{
		custom_meta_type<app::input::OnKeyboardButtonUp>("OnKeyboardButtonUp"_hs)
			.base<app::input::KeyboardButtonEvent>()
		;
	}
	
	template <>
	void reflect<app::input::OnKeyboardAnalogInput>()
	{
		custom_meta_type<app::input::OnKeyboardAnalogInput>("OnKeyboardAnalogInput"_hs)
			.base<app::input::KeyboardStateEvent>()

			.data<&app::input::OnKeyboardAnalogInput::analog>("analog"_hs)
			.data<&app::input::OnKeyboardAnalogInput::value>("value"_hs)

			.data<nullptr, &app::input::OnKeyboardAnalogInput::angle>("angle"_hs)
		;
	}

	template <>
	void reflect<app::input::GamepadDeadZone>()
	{
		custom_meta_type<app::input::GamepadDeadZone>("GamepadDeadZone"_hs)
			.data<&app::input::GamepadDeadZone::left_analog>("left_analog"_hs)
			.data<&app::input::GamepadDeadZone::right_analog>("right_analog"_hs)
			.data<&app::input::GamepadDeadZone::triggers>("triggers"_hs)
			//.data<&app::input::GamepadDeadZone::dpad>("dpad"_hs)

			.func<static_cast<const app::input::GamepadDeadZone::Analog* (app::input::GamepadDeadZone::*)(app::input::GamepadAnalog) const>(&app::input::GamepadDeadZone::get_analog)>("get_analog"_hs)

			/*
			.data<&app::input::GamepadDeadZone::AXIS_RANGE>("AXIS_RANGE"_hs)
			.data<&app::input::GamepadDeadZone::MIN_AXIS_VALUE>("MIN_AXIS_VALUE"_hs)
			.data<&app::input::GamepadDeadZone::MIN_AXIS_VALUE_F>("MIN_AXIS_VALUE_F"_hs)
			.data<&app::input::GamepadDeadZone::MAX_AXIS_VALUE>("MAX_AXIS_VALUE"_hs)
			.data<&app::input::GamepadDeadZone::MAX_AXIS_VALUE_F>("MAX_AXIS_VALUE_F"_hs)
			.data<&app::input::GamepadDeadZone::MIN_AXIS_VALUE_NORMALIZED>("MIN_AXIS_VALUE_NORMALIZED"_hs)
			.data<&app::input::GamepadDeadZone::MAX_AXIS_VALUE_NORMALIZED>("MAX_AXIS_VALUE_NORMALIZED"_hs)
			*/
		;

		custom_meta_type<app::input::GamepadDeadZone::Analog>("GamepadDeadZone::Analog"_hs)
			.data<&app::input::GamepadDeadZone::Analog::x>("x"_hs)
			.data<&app::input::GamepadDeadZone::Analog::y>("y"_hs)
		;
	}

	template <>
	void reflect<InputSystem>()
	{
		// Primitives:
		
		// TODO: Change this into something the game instance owns.
		reflect<Button>();

		// Components:
		reflect<InputComponent>();
		
		// Events:
		reflect<InputEvent>();
		reflect<OnInput>();
		reflect<ButtonEvent>();
		reflect<OnButtonPressed>();
		reflect<OnButtonReleased>();
		reflect<OnButtonDown>(); // OnButtonHeld
		reflect<OnAnalogInput>();

		// Raw device states:
		reflect<app::input::MouseState>();
		reflect<app::input::GamepadState>();
		reflect<app::input::KeyboardState>();

		// Raw Events:
		reflect<OnKeyboardState>();
		reflect<OnMouseState>();

		reflect<app::input::MouseStateEvent>();
		reflect<app::input::MouseButtonEvent>();
		reflect<app::input::MouseAnalogEvent>();
		reflect<app::input::OnMouseButtonDown>();
		reflect<app::input::OnMouseButtonUp>();
		reflect<app::input::OnMouseMove>();
		reflect<app::input::OnMouseScroll>();
		reflect<app::input::OnMouseVirtualAnalogInput>();

		reflect<app::input::GamepadStateEvent>();
		reflect<app::input::GamepadButtonEvent>();
		reflect<app::input::OnGamepadConnected>();
		reflect<app::input::OnGamepadDisconnected>();
		reflect<app::input::OnGamepadButtonDown>();
		reflect<app::input::OnGamepadButtonUp>();
		reflect<app::input::OnGamepadAnalogInput>();

		reflect<app::input::KeyboardStateEvent>();
		reflect<app::input::KeyboardButtonEvent>();
		reflect<app::input::OnKeyboardButtonDown>();
		reflect<app::input::OnKeyboardButtonUp>();
		reflect<app::input::OnKeyboardAnalogInput>();

		// Miscellaneous:
		reflect<app::input::GamepadDeadZone>();

		reflect_enum<app::input::MouseMotion>("MouseMotion"_hs);
		reflect_enum<app::input::KeyboardButton>("KeyboardButton"_hs);
		reflect_enum<app::input::GamepadDPadDirection>("GamepadDPadDirection"_hs);
		reflect_enum<app::input::GamepadButtonBits>("GamepadButtonBits"_hs);
		reflect_enum<app::input::GamepadAnalog>("GamepadAnalog"_hs);
	}
}