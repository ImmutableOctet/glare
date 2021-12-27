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
	class Texture;

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

			void bind_texture(const Texture& texture, const std::string& name);
			void bind_texture(const Texture& texture, std::string_view name);

			inline void bind_texture(std::tuple<std::string_view, const Texture*> tdata)
			{
				const auto& name = std::get<0>(tdata);
				const auto* texture = std::get<1>(tdata);

				bind_texture(*texture, name);
			}

			// TODO: Optimize uniform assignment.
			template <typename TextureArrayType>
			inline void bind_textures(const TextureArrayType& textures, const std::string& name, bool account_for_single_texture=true)
			{
				auto n_textures = textures.size();

				if (n_textures == 0)
				{
					return;
				}

				if (account_for_single_texture)
				{
					if (n_textures == 1)
					{
						const auto t_ptr = (textures[0]); // auto

						ASSERT(t_ptr);

						bind_texture(*t_ptr, name);

						return;
					}
				}
				
				auto idx = 0;

				for (const auto& t : textures)
				{
					ASSERT(t);

					bind_texture(*t, name + "[" + std::to_string(idx) + "]");

					idx++;
				}
			}

			inline void bind_textures(const NamedTextureArrayRaw& tdata)
			{
				for (const auto& kv : tdata)
				{
					const auto& name = kv.first;
					const auto& textures = kv.second;

					bind_textures(textures, name);
				}
			}

			template <typename pred>
			inline void bind_textures(const NamedTextureArrayRaw& tdata, pred&& pred_fn, bool account_for_single_texture=true)
			{
				for (const auto& kv : tdata)
				{
					const auto& name = kv.first;
					const auto& textures = kv.second;

					if (pred_fn(name, textures))
					{
						bind_textures(textures, name, account_for_single_texture);
					}
				}
			}

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

				const auto& material_uniforms = material.get_uniforms();

				for (const auto& uniform : material_uniforms)
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

			void draw
			(
				Model& model,
				// const math::Matrix& model_matrix,

				const graphics::ColorRGBA& color={1.0f, 1.0f, 1.0f, 1.0f},
				DrawMode draw_mode=DrawMode::All,
				
				bool auto_clear_textures=false,

				std::optional<NamedTextureGroupRaw> dynamic_textures=std::nullopt
			);
		private:
			// Returns 'true' if the final diffuse is acceptable.
			// 'has_diffuse_out' is set based on whether 'uniforms' has a valid diffuse-color.
			// The 'diffuse_color' uniform will only be updated if the 'transparency_mode' variable matches the return-value.
			bool handle_diffuse(const UniformMap& uniforms, const ColorRGBA& color, DrawMode draw_mode, bool& is_transparent);

			ref<Context> context;

			//std::unordered_map<const weak_ref<Material>, std::vector<const ref<Mesh>>> draw_operations;
	};
}