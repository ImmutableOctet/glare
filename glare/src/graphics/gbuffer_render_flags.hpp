#pragma once

#include <util/enum_operators.hpp>

#include <cstdint>

namespace graphics
{
	enum class GBufferRenderFlags : std::uint8_t // std::uint32_t
	{
		ShadowMap = (1 << 0),
		Lighting  = (1 << 1),

		All = 255,
		Default = All,
	};

	FLAG_ENUM(std::uint8_t, GBufferRenderFlags);
}