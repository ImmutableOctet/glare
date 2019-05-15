#include "context.hpp"
#include "window.hpp"
#include "canvas.hpp"

#include "native/opengl.hpp"

#include <sdl2/SDL_video.h>

namespace graphics
{
	context::context(window& wnd, backend gfx)
		: graphic_backend(gfx)
	{
		switch (gfx) // graphics_backend
		{
			case backend::OpenGL:
				gl_context = SDL_GL_CreateContext(wnd.get_handle());

				glewInit();

				break;
		}
	}

	context::~context()
	{
		switch (graphic_backend)
		{
			case backend::OpenGL:
				SDL_GL_DeleteContext(gl_context);

				break;
		}
	}

	void context::flip(window& wnd)
	{
		SDL_GL_SwapWindow(wnd.get_handle());
	}
}