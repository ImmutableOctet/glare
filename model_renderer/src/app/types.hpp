#pragma once

#include <types.hpp>

namespace app
{
	enum class WindowFlags : std::uint32_t
	{
		OpenGL,
		Direct3D,

		Default = OpenGL,
	};
}