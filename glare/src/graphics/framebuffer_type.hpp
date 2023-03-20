#pragma once

#include <util/enum_operators.hpp>

#include <cstdint>

namespace graphics
{
	enum class FrameBufferType : std::uint32_t
	{
		Unknown = (1 << 0),
		Read    = (1 << 1),
		Write   = (1 << 2),

		ReadWrite = (Read | Write),
	};
	
	FLAG_ENUM(std::uint32_t, FrameBufferType);
}