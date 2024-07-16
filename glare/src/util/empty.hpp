#pragma once

namespace util
{
	struct empty_t {};

	template <typename T>
	struct empty_type
	{
		using type = T;
	};
}