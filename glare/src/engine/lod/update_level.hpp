#pragma once

#include "types.hpp"

namespace engine
{
	enum class UpdateLevel : UpdateLevelRaw
	{
		StaticContent,
		SemiStaticContent,

		Distant,

		Fixed,
		Half,

		VeryLow,
		Low,
		Medium,
		High,
		VeryHigh,

		Count,

		Min = StaticContent,
		Max = VeryHigh,

		Realtime = VeryHigh,
		OnDemand = StaticContent,
		Never = StaticContent,

		Default = Medium,
	};

	static_assert(static_cast<UpdateLevelRaw>(UpdateLevel::Count) == (static_cast<UpdateLevelRaw>(UpdateLevel::Max) - static_cast<UpdateLevelRaw>(UpdateLevel::Min) + static_cast<UpdateLevelRaw>(1)));
}