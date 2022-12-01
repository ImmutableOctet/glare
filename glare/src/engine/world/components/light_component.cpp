#include "light_component.hpp"

#include <util/string.hpp>

namespace engine
{
	float PointLightComponent::get_radius(const LightProperties& properties, float constant) const
	{
		return get_radius(properties.max_brightness(), constant);
	}

	LightType LightComponent::resolve_light_mode(const std::string& mode)
	{
		auto m = util::lowercase(mode);

		if (m.starts_with("direction"))
		{
			return LightType::Directional;
		}

		if (m.starts_with("spot"))
		{
			return LightType::Spotlight;
		}

		/*
		if (m.starts_with("point"))
		{
			return LightType::Point;
		}
		*/

		return LightType::Point;
	}
}