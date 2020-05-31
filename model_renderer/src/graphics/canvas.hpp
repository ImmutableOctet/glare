#pragma once

/*
	TODO:
		* Add scissor functionality for rendering portions of the screen.
*/

#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>

#include <util/memory.hpp>
#include <math/math.hpp>

#include "types.hpp"

#include "material.hpp"

#include "context.hpp"

namespace app
{
	class Window;
}

namespace graphics
{
	class Context;
	class Model;
	class Shader;

	class Canvas
	{
		public:
			using DrawMode = CanvasDrawMode;

			Canvas();
			Canvas(memory::pass_ref<Context> ctx);

			~Canvas();

			inline Context& get_context() { return *context; }

			bool attach(memory::pass_ref<Context> ctx);
			void detach();

			void flip(app::Window& wnd);
			void clear(float red, float green, float blue, float alpha);

			void bind_textures(const TextureArray& textures);
			void bind_texture(const Texture& texture);

			// Retrieves the currently bound 'Shader' object.
			Shader& get_shader();

			bool update_diffuse_color(const graphics::ColorRGBA& color);

			// Binds the values of a 'Material' object, excluding its textures.
			// 'fn' takes in an 'std::string_view' as the uniform name, a 'UniformData' object as the data,
			// and a boolean reference as a control-flow mechanism.
			// and returns an 'std::optional<UniformData>' object.
			// If the return-value is not 'std::nullopt', it will be returned will be used to update the uniform.
			// If the return-value is 'std::nullopt', then the uniform will not be updated.
			template <typename ViewFn>
			bool bind_material_values(const Material& material, ViewFn fn)
			{
				auto& ctx = get_context();
				auto& shader = get_shader();

				bool status = true;

				for (const auto& uniform : material.get_uniforms())
				{
					const auto& name = uniform.first;
					const auto& data_raw = uniform.second;

					auto data = fn(name, data_raw, status);

					if (!status)
					{
						return false; // break;
					}

					if (data == std::nullopt)
					{
						continue;
					}

					if (!ctx.set_uniform(shader, name, *data))
					{
						return false;
					}
				}

				return true; // status
			}

			void draw(Model& model, const graphics::ColorRGBA& color={1.0f, 1.0f, 1.0f, 1.0f}, DrawMode draw_mode=DrawMode::All); // const math::Matrix& model_matrix
		private:
			// Returns 'true' if the final diffuse is acceptable.
			// 'has_diffuse_out' is set based on whether 'uniforms' has a valid diffuse-color.
			// The 'diffuse_color' uniform will only be updated if the 'transparency_mode' variable matches the return-value.
			bool handle_diffuse(const UniformMap& uniforms, const ColorRGBA& color, DrawMode draw_mode, bool& is_transparent);

			ref<Context> context;

			//std::unordered_map<const weak_ref<Material>, std::vector<const ref<Mesh>>> draw_operations;
	};
}