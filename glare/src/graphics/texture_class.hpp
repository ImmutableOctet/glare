#pragma once

#include <cstdint>

namespace graphics
{
	enum class TextureClass : std::int32_t // int
	{
		None = 0,
		Diffuse,
		Specular,
		Ambient,
		Emissive,
		Height,
		Normals,
		Shininess,
		Opacity,
		Displacement,
		Lightmap,
		Reflection,
		BaseColor,
		NormalCamera,
		EmissionColor,
		Metalness,
		DiffuseRoughness,
		AmbientOcclusion,
		Unknown,

		MaxTypes,

		//StartType = 1,
		StartType = Diffuse,
		EndType = MaxTypes
	};

	template <typename Callback>
	void enumerate_texture_classes(Callback&& fn)
	{
		for (auto texture_class_idx = static_cast<std::int32_t>(TextureClass::StartType); texture_class_idx < static_cast<std::int32_t>(TextureClass::EndType); texture_class_idx++)
		{
			const auto texture_class = static_cast<TextureClass>(texture_class_idx);

			fn(texture_class);
		}
	}
}