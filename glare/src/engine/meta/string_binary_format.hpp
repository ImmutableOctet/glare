#pragma once

#include <cstdint>

namespace engine
{
	enum class StringBinaryFormat : std::uint8_t
	{
		UTF8  = 0,
		UTF16 = 1,
		UTF32 = 2,

		Default = UTF8
	};
}