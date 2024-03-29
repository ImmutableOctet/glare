#include "mouse.hpp"
#include "mouse_events.hpp"
#include "mouse_motion.hpp"
#include "profile_metadata.hpp"

#include "input_profile_impl.hpp"
#include "input_device_impl.hpp"

#include <util/json.hpp>

#include <util/magic_enum.hpp>

#include <entt/signal/dispatcher.hpp>

#include <sdl2/SDL_hints.h>
#include <sdl2/SDL_mouse.h>

// Debugging related:
#include <util/log.hpp>

namespace app::input
{
	// Internal loading routines for mouse profiles:
	

	// Mouse:
	Mouse::Mouse(bool locked, bool use_sdl_events)
	{
		if (locked)
		{
			lock();
		}
	}

	bool Mouse::process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler)
	{
		switch (e.type)
		{
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			{
				if (event_buttons)
				{
					// TODO: Add some form of range-check for `MouseButton` cast.
					const auto mouse_button = static_cast<MouseButton>(e.button.button);
					const auto mouse_device_id = static_cast<MouseDeviceIndex>(e.button.which);

					const auto value = (e.type == SDL_MOUSEBUTTONDOWN);

					set_button(mouse_button, value);

					if (opt_event_handler)
					{
						trigger_mouse_button_event(*opt_event_handler, mouse_button, value, mouse_device_id);
					}

					return true;
				}

				break;
			}
			case SDL_MOUSEMOTION:
			{
				const auto mouse_device_id = static_cast<MouseDeviceIndex>(e.motion.which);

				bool next_state_updated = false;

				if (event_motion)
				{
					const auto [x_motion, y_motion] = process_analog_value({ e.motion.xrel, e.motion.yrel });

					next_state.x = x_motion;
					next_state.y = y_motion;
					
					next_state_updated = true;

					if (opt_event_handler)
					{
						trigger_mouse_motion_event(*opt_event_handler, x_motion, y_motion, mouse_device_id);
					}
				}

				if (event_position)
				{
					const auto position_x = e.motion.x;
					const auto position_y = e.motion.y;

					next_state.position_x = position_x;
					next_state.position_y = position_y;

					next_state_updated = true;

					mouse_position_changed = true;

					if (opt_event_handler)
					{
						trigger_mouse_position_event(*opt_event_handler, position_x, position_y, mouse_device_id);
					}
				}

				if (next_state_updated)
				{
					return true;
				}

				break;
			}
			case SDL_MOUSEWHEEL:
			{
				if (event_wheel)
				{
					const auto mouse_device_id = static_cast<MouseDeviceIndex>(e.wheel.which);

					int wheel_x = 0;
					int wheel_y = 0;

					switch (e.wheel.direction)
					{
						case SDL_MOUSEWHEEL_FLIPPED:
							wheel_x = -e.wheel.x;
							wheel_y = -e.wheel.y;

							break;

						//case SDL_MOUSEWHEEL_NORMAL:
						default:
							wheel_x = e.wheel.x;
							wheel_y = e.wheel.y;

							break;
					}

					next_state.wheel_x = wheel_x;
					next_state.wheel_y = wheel_y;

					if (opt_event_handler)
					{
						trigger_mouse_scroll_event(*opt_event_handler, wheel_x, wheel_y, mouse_device_id);
					}

					return true;
				}

				break;
			}
		}

		return false;
	}

	const Mouse::State& Mouse::poll(entt::dispatcher* opt_event_handler)
	{
		const bool manual_motion   = (!event_motion);
		const bool manual_buttons  = (!event_buttons);
		const bool manual_wheel    = (!event_wheel);
		const bool manual_position = (!event_position);

		poll_next_state(manual_motion, manual_buttons, manual_wheel, manual_position, false);

		if (opt_event_handler)
		{
			handle_manual_event_detection
			(
				*opt_event_handler,
				manual_motion, manual_buttons, manual_wheel, manual_position
			);

			handle_hat_event_detection(*opt_event_handler, next_state);
		}

		return InputDevice<MouseState>::poll(opt_event_handler);
	}

	void Mouse::peek(State& state) const
	{
		state = next_state;
	}

	void Mouse::flush()
	{
		if (event_motion)
		{
			next_state.x = 0;
			next_state.y = 0;
		}

		if (event_wheel)
		{
			next_state.wheel_x = 0;
			next_state.wheel_y = 0;
		}

		mouse_position_changed = false;
	}

	std::string Mouse::get_device_name() const
	{
		return std::string(peek_device_name());
	}

	std::string_view Mouse::peek_device_name() const
	{
		// Currently hard-coded to be "Mouse".
		// TODO: It may be worth looking into mouse-specific configurations/profiles.
		// (Not sure if SDL supports querying mouse device names)
		return "Mouse";
	}

	const MouseProfile* Mouse::get_profile() const
	{
		if (device_profile)
		{
			return &(device_profile.value());
		}

		return nullptr;
	}

	// TODO: Look into the ability to manage multiple mouse profiles.
	const MouseProfile* Mouse::load_profile(const ProfileMetadata& profile_metadata)
	{
		return input_profile_impl::load_profile
		(
			profile_metadata,
			this->device_profile,
			"mice",
			this->peek_device_name()
		);
	}

	MouseAnalogInput Mouse::process_analog_value(MouseAnalogInput raw_input) const
	{
		if (device_profile)
		{
			return device_profile->get_analog_value(raw_input);
		}

		return raw_input;
	}

	void Mouse::clear_state()
	{
		this->next_state = {};
	}

	bool Mouse::lock()
	{
		if (unlocked())
		{
			SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
			
			//SDL_CaptureMouse(SDL_TRUE);

			this->is_locked = true;

			return true;
		}

		return false;
	}

	bool Mouse::unlock()
	{
		if (locked())
		{
			SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "0", SDL_HINT_OVERRIDE);
			SDL_SetRelativeMouseMode(SDL_FALSE);
			
			//SDL_CaptureMouse(SDL_FALSE);

			this->is_locked = false;

			return true;
		}

		return false;
	}

	bool Mouse::set_lock(bool lock_state)
	{
		if (lock_state)
		{
			return lock();
		}

		return unlock();
	}

	bool Mouse::get_button(MouseButton button) const
	{
		switch (button)
		{
			case MouseButton::Left:
				return next_state.left;
			case MouseButton::Middle:
				return next_state.middle;
			case MouseButton::Right:
				return next_state.right;
			case MouseButton::Forward:
				return next_state.forward;
			case MouseButton::Back:
				return next_state.back;
		}

		return false;
	}

	void Mouse::set_button(MouseButton button, bool value)
	{
		switch (button)
		{
			case MouseButton::Left:
				next_state.left = value;

				break;
			case MouseButton::Middle:
				next_state.middle = value;

				break;
			case MouseButton::Right:
				next_state.right = value;

				break;
			case MouseButton::Forward:
				next_state.forward = value;

				break;
			case MouseButton::Back:
				next_state.back = value;

				break;
		}
	}

	Mouse::State& Mouse::poll_next_state
	(
		bool update_motion, bool update_buttons,
		bool update_wheel, bool update_position,
		bool force_clear
	) const
	{
		if (force_clear)
		{
			next_state = {}; // clear_state();
		}

		int x_motion_raw = 0;
		int y_motion_raw = 0;
		
		auto buttons = SDL_GetRelativeMouseState(&x_motion_raw, &y_motion_raw);

		if (update_motion)
		{
			const auto [x_motion, y_motion] = process_analog_value({ x_motion_raw, y_motion_raw });

			next_state.x = x_motion;
			next_state.y = y_motion;
		}

		if (update_buttons)
		{
			next_state.left    = (buttons & SDL_BUTTON(1));
			next_state.middle  = (buttons & SDL_BUTTON(2));
			next_state.right   = (buttons & SDL_BUTTON(3));
			next_state.back    = (buttons & SDL_BUTTON(4));
			next_state.forward = (buttons & SDL_BUTTON(5));
		}

		// NOTE: Wheel/scroll motion is currently unsupported for manual polling:
		if (update_wheel)
		{
			next_state.wheel_x = 0;
			next_state.wheel_y = 0;
		}

		if (update_position)
		{
			int position_x = 0;
			int position_y = 0;

			SDL_GetMouseState(&position_x, &position_y);

			const auto prev_position_x = next_state.position_x;
			const auto prev_position_y = next_state.position_y;

			if ((position_x != prev_position_x) || (position_y != prev_position_y))
			{
				next_state.position_x = position_x;
				next_state.position_y = position_y;

				mouse_position_changed = true;
			}
		}

		return next_state;
	}

	// Handles triggering mouse events when SDL events are disabled.
	bool Mouse::handle_manual_event_detection
	(
		entt::dispatcher& event_handler,
		bool check_motion, bool check_buttons, bool check_wheel, bool check_position
	) const
	{
		const auto& state = get_state();

		bool change_detected = false;

		if (check_motion)
		{
			if ((next_state.x != 0) || (next_state.y != 0))
			{
				trigger_mouse_motion_event(event_handler, next_state.x, next_state.y);

				change_detected = true;
			}
		}

		if (check_buttons)
		{
			magic_enum::enum_for_each<MouseButton>([&](MouseButton button)
			{
				auto next     = next_state.get_button(button);
				auto previous = state.get_button(button);

				if ((next != previous) || ((poll_continuous_button_down) && (next)))
				{
					trigger_mouse_button_event(event_handler, button, next);

					change_detected = true;
				}
			});
		}

		if (check_wheel)
		{
			if ((next_state.wheel_x != 0) || (next_state.wheel_y != 0))
			{
				trigger_mouse_scroll_event(event_handler, next_state.wheel_x, next_state.wheel_y);

				change_detected = true;
			}
		}

		if (check_position)
		{
			if (mouse_position_changed)
			{
				trigger_mouse_position_event(event_handler, next_state.position_x, next_state.position_y);

				//mouse_position_changed = false;

				change_detected = true;
			}
		}

		// Notify the caller if a state change was detected.
		return change_detected;
	}

	void Mouse::handle_hat_event_detection(entt::dispatcher& event_handler, State& state, MouseDeviceIndex device_index) const
	{
		if (!device_profile)
		{
			return;
		}

		handle_hat_event_detection_impl<MouseMotion>
		(
			*device_profile, state,

			// get_button_state:
			[](const auto& state, const auto& button)
			{
				return state.get_button(button);
			},

			[this, &event_handler, &device_index](const auto& state, const auto& analog, const auto& input_direction)
			{
				event_handler.enqueue<OnMouseVirtualAnalogInput>
				(
					device_index,
					state,
					analog,
					input_direction
				);
			}
		);
	}

	void Mouse::trigger_mouse_button_event
	(
		entt::dispatcher& event_handler,
		MouseButton button,
		bool is_down,
		MouseDeviceIndex device_index
	) const
	{
		if (is_down)
		{
			event_handler.enqueue<OnMouseButtonDown>
			(
				device_index,
				next_state,
				button
			);
		}
		else
		{
			event_handler.enqueue<OnMouseButtonUp>
			(
				device_index,
				next_state,
				button
			);
		}
	}

	void Mouse::trigger_mouse_motion_event
	(
		entt::dispatcher& event_handler,
		int x, int y,
		MouseDeviceIndex device_index
	) const
	{
		event_handler.enqueue<OnMouseMove>
		(
			device_index,
			next_state,
			MouseMotion::Movement,
			x, y
		);
	}

	void Mouse::trigger_mouse_scroll_event
	(
		entt::dispatcher& event_handler,
		int wheel_x, int wheel_y,
		MouseDeviceIndex device_index
	) const
	{
		event_handler.enqueue<OnMouseScroll>
		(
			device_index,
			next_state,
			MouseMotion::Scroll,
			wheel_x, wheel_y
		);
	}

	void Mouse::trigger_mouse_position_event
	(
		entt::dispatcher& event_handler,
		int x, int y,
		MouseDeviceIndex device_index
	) const
	{
		event_handler.enqueue<OnMousePosition>
		(
			device_index,
			next_state,
			x, y
		);
	}
}