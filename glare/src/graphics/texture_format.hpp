#pragma once

//#include <cstdint>

namespace graphics
{
	enum class TextureFormat
	{
		Unknown = 0,

		R,
		RG,
		RGB,
		RGBA,

		// Context sensitive; equivalent to the driver's default depth-format.
		Depth,

		// Context sensitive; equivalent to the driver's default stencil-format.
		Stencil,

		// Context sensitive; equivalent to a depth + stencil buffer.
		DepthStencil,
	};
}