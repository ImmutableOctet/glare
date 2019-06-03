#pragma once

#include "Window.hpp"

#include <sdl2/SDL_video.h>
#include <sdl2/SDL_events.h>

namespace app
{
	Window::Window(int width, int height, const std::string& title)
	{
		// TODO: Graphics Abstraction.
		handle = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
	}

	Window::~Window()
	{
		if (handle)
		{
			SDL_DestroyWindow(handle);
			//handle = null;
		}
	}

	bool Window::handle_events()
	{
		SDL_Event event;

		while (SDL_PollEvent(&event)) // SDL_WaitEvent
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					break;
				case SDL_QUIT:
					return false;
			}
		}

		return true;
	}
}