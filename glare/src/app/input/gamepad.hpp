#pragma once

#include "types.hpp"
#include "input_device.hpp"
#include "gamepad_state.hpp"
#include "gamepad_deadzone.hpp"

#include <sdl2/SDL_joystick.h>

#include <string>
#include <string_view>

//struct _SDL_Joystick;
//typedef struct _SDL_Joystick SDL_Joystick;

namespace app::input
{
	struct GamepadProfile;

	class Gamepad : public InputDevice<GamepadState>
	{
		public:
			using DeviceIndex = GamepadDeviceIndex;
			using DeadZone    = GamepadDeadZone;
		
		private:
			inline Gamepad& operator=(const Gamepad& gamepad) noexcept = default;
			Gamepad(const Gamepad&) noexcept = default;
		protected:
			DeviceIndex device_index;
			DeadZone deadzone;

			SDL_Joystick* handle;

			State next_state;
		public:
			// NOTE: `Gamepad` objects are not open immediately by default.
			Gamepad(DeviceIndex device_index=0, const DeadZone& deadzone={}, bool open_immediately=false);
			~Gamepad();
			
			inline Gamepad(Gamepad&& gamepad) noexcept
				: Gamepad(gamepad) // <-- Execute copy constructor.
			{
				// Since we now own `gamepad`'s `handle` value, set it to `nullptr`.
				gamepad.handle = nullptr;
			}

			inline Gamepad& operator=(Gamepad&& gamepad) noexcept
			{
				// Copy-assignment.
				*this = gamepad;

				// Since we now own `gamepad`'s `handle` value, set it to `nullptr`.
				gamepad.handle = nullptr;

				return *this;
			}

			void apply_profile(const GamepadProfile& profile);

			inline bool is_open()   const { return (handle); }
			inline bool is_closed() const { return !is_open(); }

			// The last device index established for this gamepad.
			inline DeviceIndex index() const { return device_index; }

			// Opens a handle to the `device_index` this gamepad was last assigned.
			// See other overload for more details on `force` parameter.
			bool open(bool force=false);

			// Opens a handle to a gamepad at index `device_index`.
			// If a gamepad handle has already been opened, this will close the previous handle.
			// The `force` argument indicates whether a forced closure will take place on the existing handle.
			bool open(DeviceIndex device_index, bool force=false);

			// Closes the current gamepad handle.
			bool close();

			const DeadZone& get_deadzone() const;
			void set_deadzone(const DeadZone& deadzone);

			// Returns the name of the underlying device.
			// If a handle to the device has not been opened,
			// this will return an empty string.
			std::string get_device_name() const;

			// Optimized version of `get_device_name`. (Avoids allocations)
			std::string_view get_device_name_as_view() const;

			virtual void peek(State& state) const override;
			virtual bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr) override;
		protected:
			bool process_button_event(const SDL_JoyButtonEvent& e, entt::dispatcher* opt_event_handler=nullptr);
	};
}