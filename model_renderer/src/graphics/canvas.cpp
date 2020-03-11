#include "canvas.hpp"
#include "context.hpp"
#include "model.hpp"

namespace graphics
{
	Canvas::Canvas()
	{
		// Nothing so far.
	}

	Canvas::Canvas(memory::pass_ref<Context> context)
		: Canvas()
	{
		attach(context);
	}

	Canvas::~Canvas()
	{
		detach();
	}

	bool Canvas::attach(memory::pass_ref<Context> ctx)
	{
		if (ctx == nullptr)
		{
			return false;
		}

		this->context = ctx;

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
	
	void Canvas::draw(Model& model, const math::Matrix& model_matrix)
	{
		for (auto& m : model.get_meshes())
		{
			auto& mesh = *m.first;

			context->use(mesh, [&]()
			{
				context->draw();
			});
		}
	}
}