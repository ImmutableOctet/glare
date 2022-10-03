#pragma once

#include "buttons.hpp"
#include "analogs.hpp"

namespace engine
{
	struct InputState
	{
		struct ButtonStates
		{
			union
			{
				ButtonsRaw bits;

				// Boolean values representing the buttons found in the `Button` enum.
				//
				// NOTE: Technically UB according to the C++ standard,
				// but supported on most platforms anyway:
				struct
				{
					// Button             // Bit
					bool Jump         : 1; // 1
					bool HeavyAttack  : 1; // 2
					bool Interact     : 1; // 3
					bool MediumAttack : 1; // 4

					// TODO: Change to a different bit.
					bool Shield : 1; // 5

					// TODO: Change to a different bit.
					bool FirstPerson : 1; // 6
				};
			};

			bool get_button(Button button) const;
			void set_button(Button button, bool value);

			void clear();
		};

		using AnalogStates = InputAnalogStates;

		// Buttons pressed this frame/update.
		// (i.e. previously not pressed)
		ButtonStates pressed;

		// Buttons held for multiple frames/updates.
		ButtonStates held;

		// Buttons released this frame/update.
		ButtonStates released;

		AnalogStates prev_directional_input;
		AnalogStates directional_input;
	};
}