#pragma once

#include <util/enum_operators.hpp>

#include <cstdint>

namespace graphics
{
	enum class GBufferFlags : std::uint32_t
	{
		None = 0,

		DepthTexture = (1 << 0),
		Position = (1 << 1),
		RenderFlags = (1 << 2),

		Default = (DepthTexture | RenderFlags) // | Position
	};

	FLAG_ENUM(std::uint32_t, GBufferFlags);
}