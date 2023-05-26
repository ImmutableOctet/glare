#include "conversion.hpp"

namespace math
{
	math::vec2f to_normalized_device_coordinates_ex(const math::vec2f& half_display_size, const math::vec2f& position)
	{
		constexpr bool flip_y = true;

		auto result = ((position - half_display_size) / half_display_size);

		if constexpr (flip_y)
		{
			result.y = -result.y;
		}

		return result;
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

	math::vec2f normalized_device_coordinates_to_screen_space(const math::vec2f& normalized_position)
	{
		return
		{
			(1.0f - ((-normalized_position.x + 1.0f) / 2.0f)),
			((-normalized_position.y + 1.0f) / 2.0f)
		};
	}

	math::vec2f normalized_device_coordinates_to_screen_space(const math::vec2f& display_size, const math::vec2f& normalized_position)
	{
		return (display_size * normalized_device_coordinates_to_screen_space(normalized_position));
	}
}