#include "material.hpp"

namespace graphics
{
	bool transparent_material(const Material& material)
	{
		auto it = material.find(std::string(MATERIAL_VAR_ALPHA));

		if (it != material.end())
		{
			const auto& value_raw = it->second;

			auto value = util::get_value(value_raw, 1.0f);

			if (value <= MATERIAL_TRANSPARENCY_THRESHOLD)
			{
				return true;
			}
		}

		return false;
	}
	
	/*
	Material::Material(pass_ref<Shader> shader)
		: shader(shader) {}

	inline Shader& Material::get_shader() { return *shader; }

	ForwardMaterial::ForwardMaterial(pass_ref<Shader> shader)
		: Material(shader) {}
	*/
}