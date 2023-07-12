#pragma once

#include <cstdint>

namespace engine
{
	enum class DeltaSystemMode : std::uint8_t
	{
		FixedUpdate,
		OnDemand,

		Default = FixedUpdate,
	};
}