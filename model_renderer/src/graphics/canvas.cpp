#include "canvas.hpp"
#include "context.hpp"

// TODO: Remove this requirement.
#include "window.hpp"

#include "native/opengl.hpp"

namespace graphics
{
	canvas::canvas()
	{
		// Nothing so far.
	}

	canvas::canvas(std::shared_ptr<context> ctx)
		: canvas()
	{
		attach(ctx);
	}

	canvas::~canvas()
	{
		detach();
	}

	bool canvas::attach(std::shared_ptr<context> ctx)
	{
		if (ctx == nullptr)
		{
			return false;
		}

		ctx = std::move(ctx);

		return true;
	}

	void canvas::detach()
	{
		ctx = nullptr;
	}

	void canvas::flip(window& wnd)
	{
		ctx->flip(wnd);
	}

	void canvas::clear(float red, float green, float blue, float alpha)
	{
		// TODO: Graphics Abstraction.
		glClearColor(red, green, blue, alpha);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}