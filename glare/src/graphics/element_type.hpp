#pragma once

//#include <cstdint>

namespace graphics
{
	enum class ElementType // : std::uint8_t
	{
		Byte,
		UByte,
		Short,
		UShort,
		Int,
		UInt,
		Half,
		Float,
		Double,

		// Specialized:

		// Only used for special-case scenarios, such as 24-bit depth buffers, etc.
		UInt24,

		// Only used for 32-bit depth + 8-bit stencil. (32 + 8)
		Int32_8,

		Bit,    // 1-bit integer.
		Nibble, // 4-bit integer.

		Char = Byte, // UByte
		Unknown = -1,
	};
}