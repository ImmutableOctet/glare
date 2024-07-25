#pragma once

#include <string>

namespace engine
{
	struct PlayerName
	{
		public:
			using String = std::string; // std::wstring;

			String value;
	};
}