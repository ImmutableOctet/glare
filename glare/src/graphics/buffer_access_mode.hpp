#pragma once

#include <cstdint>

namespace graphics
{
	// Used to describe how vertex data will be accessed.
	enum class BufferAccessMode : std::uint8_t
	{
		// Static:
		StaticCopy,
		StaticDraw,
		StaticRead,
		
		// Dynamic:
		DynamicCopy,
		DynamicDraw,
		DynamicRead,

		// Stream:
		StreamCopy,
		StreamDraw,
		StreamRead,
	};
}