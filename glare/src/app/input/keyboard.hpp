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
			Keyboard();

			virtual void peek(State& state) const override;
			//bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr) override { return false; }

			std::string get_device_name() const;
			std::string_view peek_device_name() const;

			// Returns a temporary read-only pointer to the profile loaded for this mouse device.
			// If a profile has not been defined, this will return `nullptr`.
			const KeyboardProfile* get_profile() const; // std::optional<std::reference_wrapper<const KeyboardProfile>>

			// Loads a mouse device-profile using the `profile_metadata` specified.
			// If a profile already exists, and the data loaded from `profile_metadata.path` has a valid profile, this will replace it.
			// The returned value is a temporary read-only pointer to the internally held device-profile object.
			const KeyboardProfile* load_profile(const ProfileMetadata& profile_metadata);

		protected:
			// Optional profile object, used to configure device-specific parameters,
			// as well as handle button mappings for game engines, etc.
			std::optional<KeyboardProfile> device_profile;
	};
}