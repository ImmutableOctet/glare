#include "material.hpp"

namespace graphics
{
	Material::Material(pass_ref<Shader> shader)
		: shader(shader) {}

	inline Shader& Material::get_shader() { return *shader; }

	ForwardMaterial::ForwardMaterial(pass_ref<Shader> shader)
		: Material(shader) {}
}