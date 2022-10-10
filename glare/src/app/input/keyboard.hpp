#pragma once

#include "types.hpp"
#include "input_device.hpp"
#include "keyboard_state.hpp"
#include "keyboard_profile.hpp"

#include <optional>

namespace app::input
{
	struct ProfileMetadata;

	class Keyboard : public InputDevice<KeyboardState>
	{
		public:
			static constexpr KeyboardDeviceIndex DEFAULT_KEYBOARD_DEVICE_INDEX = 0;

			Keyboard(bool event_buttons=false);

			const State& poll(entt::dispatcher* opt_event_handler=nullptr) override;
			void peek(State& state) const override;
			void flush() override;

			bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr) override;

			// Handles triggering keyboard button events when SDL events are disabled.
			// The return value indicates if a state-change was detected.
			// NOTE: This function assumes `next_state` has been updated via a call to `poll_next_state` prior.
			bool handle_manual_event_detection(entt::dispatcher& event_handler, bool check_buttons) const;

			// Enumerates key-based Hat descriptors, generating `OnKeyboardAnalogInput` events appropriately.
			void handle_hat_event_detection(entt::dispatcher& event_handler, State& state, KeyboardDeviceIndex device_index=DEFAULT_KEYBOARD_DEVICE_INDEX);

			std::string get_device_name() const;
			std::string_view peek_device_name() const;

			// Returns a temporary read-only pointer to the profile loaded for this mouse device.
			// If a profile has not been defined, this will return `nullptr`.
			const KeyboardProfile* get_profile() const; // std::optional<std::reference_wrapper<const KeyboardProfile>>

			// Loads a mouse device-profile using the `profile_metadata` specified.
			// If a profile already exists, and the data loaded from `profile_metadata.path` has a valid profile, this will replace it.
			// The returned value is a temporary read-only pointer to the internally held device-profile object.
			const KeyboardProfile* load_profile(const ProfileMetadata& profile_metadata);

			/*
				Manually updates (polls) `next_state` for the elements requested.
				This is called automatically during `peek` if any of the 'event' flags is set to false.

				NOTES:
					* This is const-by-necessity (`peek` API) and does in-fact modify this object,
					although the `InputDevice::state` object is not modified.
			*/
			State& poll_next_state(bool update_buttons, bool force_clear=false) const;
		protected:
			void trigger_keyboard_button_event
			(
				entt::dispatcher& event_handler,
				KeyboardButton button,
				bool is_down,
				KeyboardDeviceIndex device_index=DEFAULT_KEYBOARD_DEVICE_INDEX
			) const;

			// Optional profile object, used to configure device-specific parameters,
			// as well as handle button mappings for game engines, etc.
			std::optional<KeyboardProfile> device_profile;

			// Mutable due to interface for `peek`.
			mutable State next_state;
		private:
			bool event_buttons : 1 = false;
	};
}