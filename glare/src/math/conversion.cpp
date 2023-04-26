#include "conversion.hpp"

namespace math
{
	math::vec2f to_normalized_device_coordinates_ex(const math::vec2f& half_display_size, const math::vec2f& position)
	{
		return ((position - half_display_size) / half_display_size);
	}

	math::vec2f to_normalized_device_coordinates(const math::vec2f& display_size, const math::vec2f& position)
	{
		return to_normalized_device_coordinates_ex((display_size * 0.5f), position);
	}

	math::vec2f from_normalized_device_coordinates_ex(const math::vec2f& half_display_size, const math::vec2f& normalized_position)
	{
		return ((normalized_position + 1.0f) * half_display_size);
	}

	math::vec2f from_normalized_device_coordinates(const math::vec2f& display_size, const math::vec2f& normalized_position)
	{
		return (((normalized_position + 1.0f) / 2.0f) * display_size);
	}
}