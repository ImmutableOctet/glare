#pragma once

//#include <string>
#include <string_view>
//#include <util/variant.hpp>
//#include <math/math.hpp>
#include "types.hpp"
//#include "shader.hpp"

namespace engine
{
	class ModelLoader;
}

namespace graphics
{
	class Context;
	class Canvas;
	class Shader;
	class Mesh;
	class Model;

	class Material
	{
		public:
			static constexpr std::string_view ALPHA = "alpha"; // sv
			static constexpr std::string_view DIFFUSE_COLOR = "diffuse_color"; // sv
			static constexpr std::string_view HEIGHT_MAP_SCALE = "height_map_scale";
			static constexpr std::string_view SHININESS = "shininess";

			static constexpr float TRANSPARENCY_THRESHOLD = 0.99f; // <=

			inline static bool value_is_transparent(float value)
			{
				return (value <= TRANSPARENCY_THRESHOLD);
			}

			inline static bool value_is_transparent(const ColorRGBA& color)
			{
				return value_is_transparent(color.a);
			}
		protected:
			friend Context;
			friend Canvas;
			friend Shader;
			friend Mesh;
			friend Model;
			friend engine::ModelLoader;

			TextureMap textures;
			UniformMap uniforms;

			ref<Shader> shader;
		public:
			Material(const Material&) = default;

			Material(pass_ref<Shader> shader);

			template <typename T>
			inline Material& set_var(const std::string_view& material_var, const T& value)
			{
				uniforms[std::string(material_var)] = value;

				return *this;
			}

			inline const UniformMap& get_uniforms() const { return uniforms; }
			inline const TextureMap& get_textures() const { return textures; }

			inline Shader& get_shader() { return *shader; }

			inline bool has_textures() const { return (textures.size() > 0); }

			// Returns whether 'material' is considered transparent or not.
			bool transparent() const;
	};

	/*
	// TODO: Break this interface into multiple material types.
	class Material
	{
		protected:
			ref<Shader> shader;
		public:
			Shader& get_shader();

			Material(pass_ref<Shader> shader);
	};

	// Forward renderer:
	class ForwardMaterial : public Material
	{
		public:
			ForwardMaterial(pass_ref<Shader> shader);
		protected:
			Color diffuse;
			Color ambient;
			Color specular;
			Color emissive;

			float shininess; // = 40.0f;
			float alpha; // = 1.0f;

			ref<Texture> color_map;
			ref<Texture> detail_map;
			ref<Texture> alpha_map;
			ref<Texture> bump_map;
	};
	*/
}