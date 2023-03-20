#pragma once

#include "types.hpp"

namespace engine
{
	struct Command
	{
		public:
			Entity source = null;
			Entity target = null;

			inline Entity entity() const
			{
				return target;
			}
		//protected:
			//bool operator==(const Command&) const noexcept = default;
			//bool operator!=(const Command&) const noexcept = default;
	};
}