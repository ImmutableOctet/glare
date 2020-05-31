#include "canvas.hpp"
#include "context.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "model.hpp"

#include "context_state.hpp"

#include <util/variant.hpp>

#include <variant>
#include <utility>
#include <string_view>

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

	void Canvas::bind_textures(const TextureArray& textures)
	{
		for (auto& t : textures)
		{
			bind_texture(*t);
		}
	}

	void Canvas::bind_texture(const Texture& texture)
	{
		context->bind(texture);
	}

	Shader& Canvas::get_shader()
	{
		auto& ctx = *context;
		auto& state = ctx.get_state();

		return state.shader;
	}

	bool Canvas::update_diffuse_color(const graphics::ColorRGBA& color)
	{
		auto& ctx = get_context();
		auto& state = ctx.get_state();
		auto& shader = (*state.shader);

		return ctx.set_uniform(shader, Material::DIFFUSE_COLOR, color);
	}
	
	void Canvas::draw(Model& model, const graphics::ColorRGBA& color, DrawMode draw_mode) // const math::Matrix& model_matrix
	{
		auto& ctx = get_context();

		auto& state = ctx.get_state();
		auto& shader = (*state.shader);

		// TODO: Add additional outer-loop for available shaders.
		
		for (auto& mesh_descriptor : model.get_meshes())
		{
			auto& material = mesh_descriptor.material;

			if (material.get_shader() != shader)
			{
				continue;
			}

			context->clear_textures(); // (!has_material)

			for (auto& texture_group : material.textures)
			{
				auto& _data = texture_group.second;

				// TODO: Implement as visit:
				if (util::peek_value<TextureArray>(_data, [&](const TextureArray& textures)
				{
					bind_textures(textures);
				})) {}
				else if (util::peek_value<ref<Texture>>(_data, [&](const ref<Texture>& texture)
				{
					bind_texture(*texture);
				})) {}
			}

			const auto& uniforms = material.get_uniforms();

			bool is_transparent = false;

			bool render_mesh = handle_diffuse(uniforms, color, draw_mode, is_transparent);

			if (!render_mesh)
			{
				continue;
			}

			bind_material_values(material, [&](std::string_view name, const UniformData& value, bool& status) -> std::optional<UniformData>
			{
				if (name == Material::DIFFUSE_COLOR)
				{
					return std::nullopt;
				}

				return value;
			});

			for (auto& mesh : mesh_descriptor.meshes)
			{
				context->use(mesh, [&]()
				{
					context->draw();
				});
			}
		}
	}

	bool Canvas::handle_diffuse(const UniformMap& uniforms, const ColorRGBA& color, DrawMode draw_mode, bool& is_transparent)
	{
		bool continue_rendering = true;

		auto it = uniforms.find(Material::DIFFUSE_COLOR);

		bool has_diffuse = (it != uniforms.end());

		auto on_color = [&](const auto& new_diffuse)
		{
			bool is_transparent = Material::value_is_transparent(new_diffuse);

			if (((draw_mode & DrawMode::Opaque) && !is_transparent) || ((draw_mode & DrawMode::Transparent) && is_transparent))
			{
				update_diffuse_color(new_diffuse);
			}
			else
			{
				continue_rendering = false;
			}
		};

		if (has_diffuse)
		{
			auto value_raw = it->second;

			// TODO: Change use of 'peek_value' into 'std::visit'.
			if (util::peek_value<graphics::ColorRGBA>(value_raw, [&](const auto& value)
			{
				on_color(color * value);
			})) {}
			else if (util::peek_value<graphics::ColorRGB>(value_raw, [&](const auto& value)
			{
				on_color(color * ColorRGBA(value.r, value.g, value.b, 1.0f));
			})) {}
		}
		else
		{
			on_color(color);
		}

		return continue_rendering;
	}
}