#pragma once

#include "window.hpp"

#include <sdl2/SDL_video.h>

namespace graphics
{
	window::window(int width, int height, const std::string& title)
	{
		// TODO: Graphics Abstraction.
		handle = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
	}

	window::~window()
	{
		if (handle)
		{
			SDL_DestroyWindow(handle);
			//handle = null;
		}
	}
}