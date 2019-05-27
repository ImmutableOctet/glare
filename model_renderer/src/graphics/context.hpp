#pragma once

// TODO: Review exposure of implementation detail.
using SDL_GLContext = void*; // class SDL_GLContext;

namespace app
{
	class Window;
}

namespace graphics
{
	class Canvas;

	enum backend
	{
		OpenGL
	};

	class Context
	{
		public:
			Context(app::Window& wnd, backend gfx);
			~Context();

			// Commands:
			void flip(app::Window& wnd);
		private:
			const backend graphic_backend;

			// TODO: Graphics abstraction -- Use std::variant or similar.
			SDL_GLContext gl_Context;
	};
}