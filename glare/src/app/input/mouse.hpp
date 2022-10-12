#pragma once

#define _ENABLE_ATOMIC_ALIGNMENT_FIX

#include "types.hpp"
#include "input_device.hpp"
#include "mouse_state.hpp"
#include "mouse_buttons.hpp"
#include "mouse_profile.hpp"

#include <string>
#include <string_view>
#include <optional>
//#include <functional>

namespace app::input
{
	struct ProfileMetadata;
	//struct MouseProfile;

	// TODO: Rework mouse button-down events to be continuous.
	class Mouse : public InputDevice<MouseState>
	{
		public:
			static constexpr MouseDeviceIndex DEFAULT_MOUSE_DEVICE_INDEX = 0;

			Mouse(bool locked=false, bool use_sdl_events=true);

			bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr) override;

			const State& poll(entt::dispatcher* opt_event_handler=nullptr) override;
			void peek(State& state) const override;
			void flush() override;

			std::string get_device_name() const;
			std::string_view peek_device_name() const;

			// Returns a temporary read-only pointer to the profile loaded for this mouse device.
			// If a profile has not been defined, this will return `nullptr`.
			const MouseProfile* get_profile() const; // std::optional<std::reference_wrapper<const MouseProfile>>

			// Loads a mouse device-profile using the `profile_metadata` specified.
			// If a profile already exists, and the data loaded from `profile_metadata.path` has a valid profile, this will replace it.
			// The returned value is a temporary read-only pointer to the internally held device-profile object.
			const MouseProfile* load_profile(const ProfileMetadata& profile_metadata);

			bool lock();
			bool unlock();

			bool set_lock(bool lock_state);

			inline bool toggle_lock() { return set_lock(!locked()); }

			inline bool locked() const   { return is_locked; }
			inline bool unlocked() const { return !locked(); }
		protected:
			// Mutable due to interface for `peek`.
			mutable State next_state;

			// Optional profile object, used to configure device-specific parameters,
			// as well as handle button mappings for game engines, etc.
			mutable std::optional<MouseProfile> device_profile;

			bool get_button(MouseButton button) const;
			void set_button(MouseButton button, bool value);

			/*
				Manually updates (polls) `next_state` for the elements requested.
				This is called automatically during `peek` if any of the 'event' flags is set to false.
				
				NOTES:
					* This is const-by-necessity (`peek` API) and does in-fact modify this object,
					although the `InputDevice::state` object is not modified.

					* Mouse wheel functionality is currently unsupported using manual polling.
					Setting `update_wheel` to true will result in zeroed-out motion values.
			*/
			State& poll_next_state
			(
				bool update_motion, bool update_buttons, bool update_wheel=false,
				bool force_clear=false
			) const;
			
			// Handles triggering mouse events when SDL events are disabled.
			// The return value indicates if a state-change was detected.
			// NOTE: This function assumes `next_state` has been updated via a call to `poll_next_state` prior.
			bool handle_manual_event_detection
			(
				entt::dispatcher& event_handler,
				bool check_motion, bool check_buttons, bool check_wheel
			) const;

			// Enumerates button-based Hat descriptors, generating `OnMouseVirtualAnalogInput` events appropriately.
			void handle_hat_event_detection(entt::dispatcher& event_handler, State& state, MouseDeviceIndex device_index=DEFAULT_MOUSE_DEVICE_INDEX) const;

			void trigger_mouse_button_event
			(
				entt::dispatcher& event_handler,
				MouseButton button,
				bool is_down,
				MouseDeviceIndex device_index=DEFAULT_MOUSE_DEVICE_INDEX
			) const;

			void trigger_mouse_motion_event
			(
				entt::dispatcher& event_handler,
				int x, int y,
				MouseDeviceIndex device_index=DEFAULT_MOUSE_DEVICE_INDEX
			) const;

			void trigger_mouse_scroll_event
			(
				entt::dispatcher& event_handler,
				int wheel_x, int wheel_y,
				MouseDeviceIndex device_index=DEFAULT_MOUSE_DEVICE_INDEX
			) const;
		private:
			bool is_locked      : 1 = false;

			bool event_motion  : 1 = false;
			bool event_buttons : 1 = true;
			bool event_wheel   : 1 = true;
	};
}