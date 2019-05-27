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

			inline SDL_Window* get_handle() { return handle; }
		private:
			SDL_Window* handle; // = nullptr;
	};
}