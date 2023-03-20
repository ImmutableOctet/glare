#pragma once

#include <util/enum_operators.hpp>

#include <cstdint>

namespace graphics
{
	// NOTE: Support for texture flags is driver-dependent.
	enum class TextureFlags : std::uint32_t
	{
		None               = (1 << 0),

		// TODO: Review the best way to handle depth and stencil data:
		//DepthMap           = (1 << 1),
		//DepthStencilMap    = (1 << 2),
		Clamp = (1 << 2),

		LinearFiltering    = (1 << 3),
		MipMap             = (1 << 4),
		WrapS              = (1 << 5),
		WrapT              = (1 << 6),

		Dynamic            = (1 << 7),
		
		WrapST             = (WrapS|WrapT),

		Default            = (LinearFiltering|WrapST),
	};
	
	FLAG_ENUM(std::uint32_t, TextureFlags);
}