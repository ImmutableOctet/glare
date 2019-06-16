#pragma once

#include <math/math.hpp>
#include "types.hpp"

#include "shader.hpp"

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

	class ForwardMaterial : public Material
	{
		public:
			ForwardMaterial(pass_ref<Shader> shader);
		protected:
			ShaderVar<Color> diffuse;
			ShaderVar<Color> ambient;
			ShaderVar<Color> specular;
			ShaderVar<Color> emissive;

			ShaderVar<float> shininess; // = 40.0f;
			ShaderVar<float> alpha; // = 1.0f;
	};
}