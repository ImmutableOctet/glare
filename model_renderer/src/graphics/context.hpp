#pragma once

// TODO: Review exposure of implementation detail.
using SDL_GLContext = void*; // class SDL_GLContext;

namespace graphics
{
	class window;
	class canvas;

	enum backend
	{
		OpenGL
	};

	class context
	{
		public:
			context(window& wnd, backend gfx);
			~context();

			// Commands:
			void flip(window& wnd);
		private:
			const backend graphic_backend;

			// TODO: Graphics abstraction -- Use std::variant or similar.
			SDL_GLContext gl_context;
	};
}