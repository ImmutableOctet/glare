#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class InputSystem;

	 //template <> extern void reflect<InputSystem>();
	extern template void reflect<InputSystem>();
}