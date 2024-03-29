#pragma once

#include <util/enum_operators.hpp>

#include <cstdint>

namespace app
{
	enum class WindowFlags : std::uint32_t
	{
		OpenGL            = (1 << 0),
		Direct3D          = (1 << 1),

		Fullscreen        = (1 << 2),
		Fullscreen_Auto   = (1 << 3),
		Shown             = (1 << 4),
		Hidden            = (1 << 5),
		Borderless        = (1 << 6),
		Resizable         = (1 << 7),
		Minimized         = (1 << 8),
		Maximized         = (1 << 9),

		// TODO: Review input-related flags:
		/*
		Input_Grabbed     = (1 << 10),
		Input_Focus       = (1 << 11),
		Mouse_Focus       = (1 << 12),
		Mouse_Capture     = (1 << 13),
		*/

		Default = (OpenGL|Resizable),
	};

	FLAG_ENUM(std::uint32_t, WindowFlags);
}