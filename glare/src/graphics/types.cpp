#include "types.hpp"

#include <math/math.hpp>

#include <cmath>

namespace graphics
{
	void PointRect::set_width(Type value)
	{
		end.x = (start.x + value);
	}

	void PointRect::set_height(Type value)
	{
		end.y = (start.y + value);
	}

	PointRect::Type PointRect::get_width() const // inline auto
	{
		return std::abs(end.x - start.x);
	}

	PointRect::Type PointRect::get_height() const // inline auto
	{
		return std::abs(end.y - start.y);
	}

	PointRect::Type PointRect::get_length() const
    {
		return static_cast<PointRect::Type>(std::sqrt(math::sq(get_width()) + math::sq(get_height())));
    }
}