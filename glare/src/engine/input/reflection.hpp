#pragma once

#include <engine/reflection.hpp>

//#include "reflection_impl_input.cpp"

namespace engine
{
	class InputSystem;

	 template <>
	 extern void reflect<InputSystem>();
}