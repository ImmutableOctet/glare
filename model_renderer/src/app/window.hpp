#pragma once

#include <string>

struct SDL_Window;

namespace app
{
	class Context;

	class Window
	{
		public:
			Window(int width, int height, const std::string& title = "");
			~Window();

			inline SDL_Window* get_handle() const { return handle; }

			// If this method returns 'false', then the window has received a 'close' message.
			// It is up to the caller of 'handle_events' to handle this message.
			bool handle_events();
		private:
			SDL_Window* handle; // = nullptr;
	};
}