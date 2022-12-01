#pragma once

#include <engine/reflection.hpp>

#include "reflection.hpp"

#include "components/input_component.hpp"

#include "buttons.hpp"
#include "events.hpp"

#include <app/input/devices.hpp>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnButtonPressed, ButtonEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnButtonReleased, ButtonEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnButtonDown, ButtonEvent); // OnButtonHeld

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
	void reflect<InputSystem>()
	{
		// Primitives:
		
		// TODO: Change this into something the game instance owns.
		reflect<Button>();

		// Components:
		reflect<InputComponent>();
		// ...

		// Events:
		reflect<InputEvent>();
		reflect<OnInput>();
		reflect<ButtonEvent>();
		reflect<OnButtonPressed>();
		reflect<OnButtonReleased>();
		reflect<OnButtonDown>(); // OnButtonHeld
		reflect<OnAnalogInput>();
	}
}