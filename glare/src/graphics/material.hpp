#pragma once

#include "types.hpp"
//#include "shader.hpp"

#include "texture_array.hpp"
#include "uniform_map.hpp"

//#include <util/variant.hpp>
//#include <math/math.hpp>

#include <unordered_map>
#include <string_view>
//#include <string>

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

	// Map of string-identifiers to 'TextureGroup' objects.
	using TextureMap = std::unordered_map<std::string, TextureGroup>; // string_view

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

			std::shared_ptr<Shader> shader;
		public:
			Material(const Material&) = default;

			Material(const std::shared_ptr<Shader>& shader);

			template <typename T>
			inline Material& set_var(std::string_view material_var, const T& value)
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
			std::shared_ptr<Shader> shader;
		public:
			Shader& get_shader();

			Material(const std::shared_ptr<Shader>& shader);
	};

	// Forward renderer:
	class ForwardMaterial : public Material
	{
		public:
			ForwardMaterial(const std::shared_ptr<Shader>& shader);
		protected:
			Color diffuse;
			Color ambient;
			Color specular;
			Color emissive;

			float shininess; // = 40.0f;
			float alpha; // = 1.0f;

			std::shared_ptr<Texture> color_map;
			std::shared_ptr<Texture> detail_map;
			std::shared_ptr<Texture> alpha_map;
			std::shared_ptr<Texture> bump_map;
	};
	*/
}