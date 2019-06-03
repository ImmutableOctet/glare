#pragma once

#include "types.hpp"

namespace app
{
	namespace input
	{
		class Mouse
		{
			public:
				struct State : public DeviceState<3>
				{
					int x, y;
				};

				Mouse();

				State get_state();
			private:
				State state;
		};
	}
}