#pragma once

#include <cstdint>

namespace engine
{
	enum class MetaVariableScope : std::uint8_t
	{
		Local,
		Global,
		Context,
		Universal,

		Count,

		Shared = Context,
	};
}