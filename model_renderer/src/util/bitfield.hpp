#pragma once

#include "bitflag.hpp"

namespace util
{
	template <typename InType, typename OutType>
	inline OutType convert_flag(InType current_in, OutType current_out, InType check_in, OutType flag_out)
	{
		if ((current_in & check_in))
		{
			current_out |= flag_out;
		}

		return current_out;
	}
}