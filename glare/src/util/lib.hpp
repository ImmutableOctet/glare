#pragma once

#include <debug.hpp>
#include <types.hpp>

#include <optional>

namespace util
{
	namespace lib
	{
		struct OpenGLVersion
		{
			using version_t = std::uint8_t;

			version_t major;
			version_t minor;
		};

		bool init_sdl();
		OpenGLVersion establish_gl(std::optional<OpenGLVersion> version=std::nullopt);
	}
}