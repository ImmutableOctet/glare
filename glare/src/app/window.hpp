#pragma once

#include <string_view>
#include <tuple>

#include "types.hpp"

// SDL:
struct SDL_Window;

// Events:
union SDL_Event;
struct SDL_WindowEvent;

namespace app
{
	class Context;
	class Application;

	class Window
	{
		public:
			using Flags = WindowFlags;

			static std::uint32_t to_native_window_flags(WindowFlags flags);

			Window(int width, int height, std::string_view title="", WindowFlags flags=WindowFlags::Default);
			~Window();

			inline SDL_Window* get_handle() const { return handle; }

			std::uint32_t get_id() const;
			WindowFlags get_flags() const;

			// `title` must be zero-terminated.
			void set_title(std::string_view title);

			// Retrieve the width and height of the window.
			void get_size(int& width, int& height) const;

			inline std::tuple<int, int> get_size() const
			{
				int width, height;

				get_size(width, height);

				return { width, height };
			}

			float horizontal_aspect_ratio() const;

			// If this method returns 'false', then the window has received a 'close' message.
			// It is up to the caller to handle the result of this message.
			bool process_event(const SDL_Event& event, const SDL_WindowEvent& window_event, Application& app);
		private:
			SDL_Window* handle; // = nullptr;
			Flags creation_flags;
	};
}