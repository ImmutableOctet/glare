#pragma once

// Common functionality shared between input devices.

#include "input_device.hpp"
#include "input_profile.hpp"

#include <math/joyhat.hpp>

namespace app::input
{
	// Handles button-detection for Hat simulation.
	// 
	// `get_button_state` is a callable taking in a reference to `state`,
	// and a button value from each of `device_profile`'s Hat descriptors.
	//
	// `on_event` is a callable that is triggered when a Hat input has been identified.
	// 
	// `on_event` takes in a reference to `state`,
	// an `AnalogType` value corresponding to the Hat,
	// and a direction-vector generated from the Hat description.
	template <typename AnalogType, typename ProfileType, typename StateType, typename GetButtonFn, typename EventFn>
	void handle_hat_event_detection_impl(ProfileType& device_profile, const StateType& state, GetButtonFn&& get_button_state, EventFn&& on_event) // const ProfileType& device_profile
	{
		std::size_t hat_index_as_analog = static_cast<std::size_t>(AnalogType::RuntimeAnalogOffset);

		// Helper lambda for dealing with `button` (optional-type).
		auto get_button = [&state, &get_button_state](const auto& button)
		{
			if (!button)
			{
				return false;
			}

			return get_button_state(state, *button);
		};

		for (auto& hat : device_profile.get_hats()) // const auto&
		{
			const auto up    = get_button(hat.up);
			const auto right = get_button(hat.right);
			const auto down  = get_button(hat.down);
			const auto left  = get_button(hat.left);

			const auto button_is_active = (up != down || left != right);

			if (!button_is_active && !hat.get_active()) // !(button_is_active || hat.get_active())
			{
				// Hat index must be incremented each loop.
				hat_index_as_analog++;

				continue;
			}

			// Retrieve the user's input direction.
			const auto input_direction = math::joyhat(up, down, left, right);

			// Convert from raw integral type to `AnalogType`.
			const auto analog = static_cast<AnalogType>(hat_index_as_analog);

			// Generate an analog event.
			on_event
			(
				state,
				analog,
				input_direction
			);
			
			// Set the Hat's activity state.
			hat.set_active(button_is_active);

			// Hat index must be incremented each loop.
			hat_index_as_analog++;
		}
	}
}