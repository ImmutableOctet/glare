#pragma once

//#include <cstdint>

namespace graphics
{
	enum class TextureType
	{
		Texture2D = 0,

		CubeMap,
		//Texture3D,

		Normal  = Texture2D,
		Default = Texture2D,
	};
}