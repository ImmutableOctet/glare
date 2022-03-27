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

	void Canvas::begin()
	{
		context->begin_frame();
	}

	void Canvas::end()
	{
		context->end_frame();
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

	void Canvas::bind_texture(const Texture& texture, const std::string& name)
	{
		context->bind(texture, name);
	}

	void Canvas::bind_texture(const Texture& texture, std::string_view name)
	{
		// Currently constructs a 'std::string' due to 'Context' requiring it.
		bind_texture(texture, std::string(name));
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
	
	void Canvas::draw
	(
		Model& model,
		// const math::Matrix& model_matrix,

		const graphics::ColorRGBA& color,
		
		DrawMode draw_mode,
		
		bool auto_clear_textures,
		
		std::optional<NamedTextureGroupRaw> dynamic_textures
	)
	{
		auto& ctx = get_context();
		auto& state = ctx.get_state();
		auto& shader = (*state.shader);
		
		for (auto& mesh_descriptor : model.get_meshes())
		{
			/*
			if (!mesh_descriptor)
			{
				continue;
			}
			*/

			auto& material = mesh_descriptor.material;

			if (!(draw_mode & DrawMode::IgnoreShaders))
			{
				if (material->get_shader() != shader)
				{
					continue;
				}
			}

			bool material_has_textures = false;
			bool texture_diffuse_enabled = false; // true;
			bool specular_available = false;
			bool normal_map_available = false;
			bool height_map_available = false;

			bool ignore_textures = (draw_mode & DrawMode::IgnoreTextures);

			if (!ignore_textures)
			{
				material_has_textures = mesh_descriptor.material->has_textures();
				//normal_map_available = mesh_descriptor.material.textures.contains(Model::get_texture_class_variable(TextureClass::Normals));

				bool force_clear_textures = (auto_clear_textures || (!material_has_textures));

				context->clear_textures(force_clear_textures); // (!has_material)

				//specular_available = false;
				//normal_map_available = false;
				//height_map_available = false;

				for (auto& texture_group : material->textures)
				{
					const auto& texture_name = texture_group.first;
					const auto& _data = texture_group.second;

					if (texture_name == Model::get_texture_class_variable(TextureClass::Diffuse))
					{
						texture_diffuse_enabled = true;
					}
					else if (texture_name == Model::get_texture_class_variable(TextureClass::Specular)) // (texture_name.find("specular") != std::string::npos)
					{
						specular_available = true;
					}
					else if (texture_name == Model::get_texture_class_variable(TextureClass::Normals))
					{
						normal_map_available = true;
					}
					else if (texture_name == Model::get_texture_class_variable(TextureClass::Height))
					{
						height_map_available = true;
					}

					// TODO: Implement as visit:
					if (util::peek_value<ref<Texture>>(_data, [&](const ref<Texture>& texture)
					{
						bind_texture(*texture, texture_name);
					}))
					{}
					else if (util::peek_value<TextureArray>(_data, [&](const TextureArray& textures)
					{
						bind_textures(textures, texture_name);
					}))
					{}
				}

				if (dynamic_textures.has_value())
				{
					//static const std::string texture_name = "depth_map"; // constexpr
					const auto& dynamic_textures_v = *dynamic_textures;

					bool shadows_enabled = !(draw_mode & DrawMode::IgnoreShadows);

					// TODO: Implement as visit:
					if (util::peek_value<std::tuple<std::string_view, const Texture*>>(dynamic_textures_v, [&](const std::tuple<std::string_view, const Texture*>& tdata)
					{
						const auto& texture_name = std::get<0>(tdata);
						const auto* texture = std::get<1>(tdata);

						if (!shadows_enabled)
						{
							if ((texture_name.find("shadow") != std::string::npos))
							{
								// Don't bind this texture.
								return; // From lambda.
							}
						}

						ASSERT(texture);
						bind_texture(*texture, texture_name); // bind_texture(tdata);
					}))
					{}
					else if (util::peek_value<const NamedTextureArrayRaw*>(dynamic_textures_v, [&](const NamedTextureArrayRaw* tdata)
					{
						//ASSERT(tdata);
						if (tdata)
						{
							bind_textures(*tdata, [shadows_enabled](const std::string& texture_name, const graphics::TextureArrayRaw& textures)
							{
								if (!shadows_enabled) // tolower(...)
								{
									if ((texture_name.find("shadow") != std::string::npos))
									{
										// Don't bind this texture.
										return false;
									}
								}

								// Bind this texture.
								return true;
							});
						}
					}))
					{}
				}
			}

			if (!(draw_mode & DrawMode::IgnoreMaterials))
			{
				const auto& uniforms = material->get_uniforms();

				bool is_transparent = false;

				// Diffuse color.
				bool render_mesh = handle_diffuse(uniforms, color, draw_mode, is_transparent);

				if (!render_mesh)
				{
					continue;
				}

				bind_material_values(*material, [&](std::string_view name, const UniformData& value, bool& status) -> std::optional<UniformData>
				{
					// Already handled above.
					if (name == Material::DIFFUSE_COLOR)
					{
						return std::nullopt;
					}

					return value;
				});

				if (!ignore_textures)
				{
					//Model::get_texture_class_variable(TextureClass::Specular)
					shader["texture_diffuse_enabled"] = texture_diffuse_enabled; // material_has_textures;
					shader["specular_available"]      = specular_available;
					shader["normal_map_available"]    = normal_map_available;
					shader["height_map_available"]    = height_map_available;
				}
			}

			///*
			bool is_cw = (model.get_winding_order() == VertexWinding::Clockwise);

			if (is_cw)
			{
				context->set_winding_order(VertexWinding::Clockwise);
			}
			//*/

			for (auto& mesh : mesh_descriptor.meshes)
			{
				context->use(mesh, [&]()
				{
					context->draw();
				});
			}

			///*
			if (is_cw)
			{
				context->set_winding_order(VertexWinding::CounterClockwise);
			}
			//*/
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