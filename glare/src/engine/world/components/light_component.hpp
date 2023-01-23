#pragma once

#include <engine/world/types.hpp>
#include <engine/world/light_properties.hpp>

namespace engine
{
	struct LightComponent : LightProperties
	{
		LightType type = LightType::Any;

		bool casts_shadows : 1 = false;

		inline bool get_casts_shadows() const { return casts_shadows; }
		inline void set_casts_shadows(bool value) { casts_shadows = value; }
	};
}