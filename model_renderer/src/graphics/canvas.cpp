#include "canvas.hpp"
#include "context.hpp"
#include "shader.hpp"
#include "model.hpp"

#include "context_state.hpp"

#include <variant>
#include <utility>

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

		context->clear_textures();
	}

	void Canvas::clear(float red, float green, float blue, float alpha)
	{
		context->clear(red, green, blue, alpha);
	}
	
	void Canvas::draw(Model& model) // const math::Matrix& model_matrix
	{
		auto& state = context->get_state();
		auto& shader = (*state.shader);

		for (auto& m : model.get_meshes())
		{
			auto& mesh     = *(m.first);
			auto& material = (m.second);

			context->clear_textures();

			for (auto& uniform : *material)
			{
				auto& _data = uniform.second;

				std::visit([&](auto&& data)
				{
					using T = std::decay_t<decltype(data)>;

					if constexpr (std::is_same_v<T, TextureArray>)
					{
						for (auto& t : data)
						{
							context->bind(*t);
						}
					}
				}, _data);
			}

			if (material != nullptr)
			{
				context->apply_uniforms(shader, *material);
			}

			context->use(mesh, [&]()
			{
				context->draw();
			});
		}
	}
}