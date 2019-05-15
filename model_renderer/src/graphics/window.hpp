#pragma once

#include <string>

struct SDL_Window;

namespace graphics
{
	class context;

	class window
	{
		public:
			window(int width, int height, const std::string& title = "");
			~window();

			inline SDL_Window* get_handle() { return handle; }
		private:
			SDL_Window* handle; // = nullptr;
	};
}