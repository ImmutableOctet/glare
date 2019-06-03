#include "Canvas.hpp"
#include "context.hpp"

namespace graphics
{
	Canvas::Canvas()
	{
		// Nothing so far.
	}

	Canvas::Canvas(std::shared_ptr<Context> ctx)
		: Canvas()
	{
		attach(ctx);
	}

	Canvas::~Canvas()
	{
		detach();
	}

	bool Canvas::attach(std::shared_ptr<Context> ctx)
	{
		if (ctx == nullptr)
		{
			return false;
		}

		ctx = std::move(ctx);

		return true;
	}

	void Canvas::detach()
	{
		ctx = nullptr;
	}

	void Canvas::flip(app::Window& wnd)
	{
		ctx->flip(wnd);
	}

	void Canvas::clear(float red, float green, float blue, float alpha)
	{
		ctx->clear(red, green, blue, alpha);
	}
}