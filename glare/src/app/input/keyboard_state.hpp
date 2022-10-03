#pragma once

#include <types.hpp>

namespace app::input
{
	struct KeyboardState
	{
		public:
			using key_value_t = std::uint8_t;

			const key_value_t* keys = nullptr;
			int num_keys = 0; // std::uint32_t

			bool get_key(int scan_code) const;
			bool has_keys() const;
	};
}