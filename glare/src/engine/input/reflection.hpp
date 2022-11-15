#pragma once

#include <engine/reflection.hpp>

#include "components/input_component.hpp"

namespace engine
{
	class InputSystem;

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
	void reflect<InputSystem>()
	{
		reflect<InputComponent>();

		// ...
	}
}