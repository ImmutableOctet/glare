#include "material.hpp"

#include <util/variant.hpp>

namespace graphics
{
	Material::Material(const std::shared_ptr<Shader>& shader)
		: shader(shader) {}

	bool Material::transparent() const
	{
		auto it = uniforms.find(std::string(ALPHA));

		if (it != uniforms.end())
		{
			const auto& value_raw = it->second;

			auto value = util::get_value(value_raw, 1.0f);

			if (value_is_transparent(value))
			{
				return true;
			}
		}

		return false;
	}
}