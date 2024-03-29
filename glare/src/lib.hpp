#pragma once

#include <cstdint>
#include <optional>

struct SDL_Window;

namespace glare
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

		bool init_imgui();
		bool deinit_imgui();

		OpenGLVersion establish_gl(std::optional<OpenGLVersion> version=std::nullopt);
	}
}