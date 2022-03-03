#pragma once

#include "lib.hpp"

#include <sdl2/SDL.h>

namespace util
{
	namespace lib
	{
		bool init_sdl()
		{
			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
			{
				SDL_Log("Failed to initialize SDL: %s", SDL_GetError());

				return false;
			}

			return true;
		}

		OpenGLVersion establish_gl(std::optional<OpenGLVersion> version)
		{
			if (!version.has_value())
				version = { 4, 3 };

			// Establish Core Profile Context:
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, version->major);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, version->minor);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

			#ifdef NDEBUG
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
			#endif

			return *version;
		}
	}
}