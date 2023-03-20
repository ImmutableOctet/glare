#pragma once

#include "point.hpp"

namespace graphics
{
	struct PointRect
	{
		//using Type = float;
		using Type = int;

		Point start = {};
		Point end = {};

		void set_width(Type value);
		void set_height(Type value);

		inline void set_size(Type width, Type height)
		{
			set_width(width);
			set_height(height);
		}

		inline void set_size(const Point& size)
		{
			set_size(size.x, size.y);
		}

		inline Type get_x() const
		{
			return start.x;
		}

		inline Type get_y() const
		{
			return start.y;
		}

		Type get_width() const; // inline auto
		Type get_height() const; // inline auto
		Type get_length() const; // inline auto
	};
}