#pragma once

#include <string>
#include <string_view>

#include <util/variant.hpp>
//#include <math/math.hpp>

#include "types.hpp"
#include "shader.hpp"

namespace graphics
{
	//class Shader;

	using Material = UniformMap;

	constexpr std::string_view MATERIAL_VAR_ALPHA = "alpha"; // sv
	constexpr std::string_view MATERIAL_VAR_DIFFUSE_COLOR = "diffuse_color"; // sv

	constexpr float MATERIAL_TRANSPARENCY_THRESHOLD = 0.99f;

	// Returns whether 'material' is considered transparent or not.
	bool transparent_material(const Material& material);

	template <typename T>
	Material& set_material_var(Material& material, std::string_view material_var, const T& value)
	{
		material[material_var] = value; // std::string(material_var)

		return material;
	}

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