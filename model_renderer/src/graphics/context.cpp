#include "Context.hpp"
#include "../app/window.hpp"
#include "Canvas.hpp"

#include "native/opengl.hpp"

#include <sdl2/SDL_video.h>

namespace graphics
{
	Context::Context(app::Window& wnd, backend gfx)
		: graphic_backend(gfx)
	{
		switch (gfx) // graphics_backend
		{
			case backend::OpenGL:
				gl_Context = SDL_GL_CreateContext(wnd.get_handle());

				glewInit();

				break;
		}
	}

	Context::~Context()
	{
		switch (graphic_backend)
		{
			case backend::OpenGL:
				SDL_GL_DeleteContext(gl_Context);

				break;
		}
	}

	void Context::flip(app::Window& wnd)
	{
		SDL_GL_SwapWindow(wnd.get_handle());
	}
}