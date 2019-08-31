#include "Canvas.hpp"
#include "context.hpp"

namespace graphics
{
	Canvas::Canvas()
	{
		// Nothing so far.
	}

	Canvas::Canvas(std::shared_ptr<Context> context)
		: Canvas()
	{
		attach(context);
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
		context = nullptr;
	}

	void Canvas::flip(app::Window& wnd)
	{
		context->flip(wnd);
	}

	void Canvas::clear(float red, float green, float blue, float alpha)
	{
		context->clear(red, green, blue, alpha);
	}
}