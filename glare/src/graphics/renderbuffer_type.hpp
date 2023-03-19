#pragma once

//#include <cstdint>

namespace graphics
{
	// Types of integrated render buffers.
	enum class RenderBufferType
	{
		Color,
		Depth,
		Stencil,
		DepthStencil,

		Unknown,
	};
}