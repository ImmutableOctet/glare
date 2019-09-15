#pragma once

#include "window.hpp"

#include <sdl2/SDL_video.h>
#include <sdl2/SDL_events.h>

namespace app
{
	Window::Window(int width, int height, const std::string& title, WindowFlags flags)
		: creation_flags(flags)
	{
		// TODO: Graphics Abstraction.
		handle = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, static_cast<std::uint32_t>(flags));
	}

	Window::~Window()
	{
		if (handle)
		{
			SDL_DestroyWindow(handle);
			//handle = null;
		}
	}

	std::uint32_t Window::get_id() const
	{
		return static_cast<std::uint32_t>(SDL_GetWindowID(get_handle()));
	}

	WindowFlags Window::get_flags() const
	{
		return creation_flags; // SDL_GetWindowFlags(...);
	}

	void Window::get_size(int& width, int& height) const
	{
		SDL_GetWindowSize(get_handle(), &width, &height);
	}

	float Window::horizontal_aspect_ratio() const
	{
		int width, height;

		get_size(width, height);

		return (static_cast<float>(width) / static_cast<float>(height));
	}

	bool Window::handle_event(const SDL_Event& event, const SDL_WindowEvent& window_event)
	{
		switch (event.window.event)
		{
			/*
			case SDL_WINDOWEVENT_RESIZED:
				auto width  = window_event.data1;
				auto height = window_event.data2;

				break;
			*/
		}

		return true;
	}
}