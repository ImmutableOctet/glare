#pragma once

//#include <math/types.hpp>

namespace engine
{
	// TODO: Rework into base `LightComponent` type.
	struct DirectionalLightComponent
	{
		//math::Vector direction = {};
		bool use_position : 1 = false;

		inline bool get_use_position() const { return use_position; }
		inline void set_use_position(bool value) { use_position = value; }
	};
}