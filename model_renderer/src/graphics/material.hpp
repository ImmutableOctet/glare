#pragma once

#include <math/math.hpp>
#include "types.hpp"

#include "shader.hpp"
#include "texture.hpp"

namespace graphics
{
	//class Shader;

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
}