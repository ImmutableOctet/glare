#pragma once

#include "window.hpp"

#include <sdl2/SDL_video.h>
#include <sdl2/SDL_events.h>

#include <util/bitfield.hpp>

namespace app
{
	std::uint32_t Window::to_native_window_flags(WindowFlags flags)
	{
		std::uint32_t sdl_flags = 0;

		auto convert = [&](WindowFlags flag, std::uint32_t native)
		{
			sdl_flags = util::convert_flag<WindowFlags, std::uint32_t>(flags, sdl_flags, flag, native);
		};

		convert(WindowFlags::OpenGL, SDL_WINDOW_OPENGL);
		convert(WindowFlags::Fullscreen, SDL_WINDOW_FULLSCREEN);
		convert(WindowFlags::Fullscreen_Auto, SDL_WINDOW_FULLSCREEN_DESKTOP);
		convert(WindowFlags::Shown, SDL_WINDOW_SHOWN);
		convert(WindowFlags::Hidden, SDL_WINDOW_HIDDEN);
		convert(WindowFlags::Borderless, SDL_WINDOW_BORDERLESS);
		convert(WindowFlags::Resizable, SDL_WINDOW_RESIZABLE);
		convert(WindowFlags::Minimized, SDL_WINDOW_MINIMIZED);
		convert(WindowFlags::Maximized, SDL_WINDOW_MAXIMIZED);

		return sdl_flags;
	}

	Window::Window(int width, int height, const std::string& title, WindowFlags flags)
		: creation_flags(flags)
	{
		// TODO: Graphics Abstraction.
		handle = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, to_native_window_flags(flags));
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
		//switch (event.window.event)
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