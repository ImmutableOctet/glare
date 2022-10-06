#include "mouse.hpp"
#include "mouse_events.hpp"

// SDL:
#include <sdl2/SDL_hints.h>
#include <sdl2/SDL_mouse.h>

// Debugging related:
#include <util/log.hpp>

namespace app::input
{
	Mouse::Mouse(bool locked, bool use_sdl_events) :
		event_motion(false),
		event_buttons(true),
		event_wheel(true)
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
				if (event_buttons)
				{
					// TODO: Add some form of range-check for `MouseButton` cast.
					auto mouse_button = static_cast<MouseButton>(e.button.button);
					auto mouse_device_id = static_cast<MouseDeviceIndex>(e.button.which);
					auto value = (e.type == SDL_MOUSEBUTTONDOWN);

					set_button(mouse_button, value);

					if (opt_event_handler)
					{
						trigger_mouse_button_event(*opt_event_handler, mouse_button, value, mouse_device_id);
					}

					return true;
				}

				break;

			case SDL_MOUSEMOTION:
				if (event_motion)
				{
					const auto mouse_device_id = static_cast<MouseDeviceIndex>(e.motion.which);

					const auto& x_motion = e.motion.xrel;
					const auto& y_motion = e.motion.yrel;

					next_state.x = x_motion;
					next_state.y = y_motion;

					if (opt_event_handler)
					{
						trigger_mouse_motion_event(*opt_event_handler, x_motion, y_motion, mouse_device_id);
					}

					return true;
				}

				break;
			case SDL_MOUSEWHEEL:
				if (event_wheel)
				{
					const auto mouse_device_id = static_cast<MouseDeviceIndex>(e.wheel.which);

					int wheel_x, wheel_y;

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

		return false;
	}

	const Mouse::State& Mouse::poll(entt::dispatcher* opt_event_handler)
	{
		bool manual_motion  = (!event_motion);
		bool manual_buttons = (!event_buttons);
		bool manual_wheel   = (!event_wheel);

		poll_next_state(manual_motion, manual_buttons, manual_wheel, false);

		if (opt_event_handler)
		{
			handle_manual_event_detection
			(
				*opt_event_handler,
				manual_motion, manual_buttons, manual_wheel
			);
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
		bool update_motion, bool update_buttons, bool update_wheel,
		bool force_clear
	) const
	{
		if (force_clear)
		{
			next_state = {};
		}

		int motion_x = 0;
		int motion_y = 0;
		
		auto buttons = SDL_GetRelativeMouseState(&motion_x, &motion_y);

		if (update_motion)
		{
			next_state.x = motion_x;
			next_state.y = motion_y;
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

		return next_state;
	}

	// Handles triggering mouse events when SDL events are disabled.
	bool Mouse::handle_manual_event_detection
	(
		entt::dispatcher& event_handler,
		bool check_motion, bool check_buttons, bool check_wheel
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
			if (next_state.left != state.left)
			{
				trigger_mouse_button_event(event_handler, MouseButton::Left, next_state.left);

				change_detected = true;
			}

			if (next_state.right != state.right)
			{
				trigger_mouse_button_event(event_handler, MouseButton::Right, next_state.right);

				change_detected = true;
			}

			if (next_state.middle != state.middle)
			{
				trigger_mouse_button_event(event_handler, MouseButton::Middle, next_state.middle);

				change_detected = true;
			}

			if (next_state.forward != state.forward)
			{
				trigger_mouse_button_event(event_handler, MouseButton::Forward, next_state.forward);

				change_detected = true;
			}

			if (next_state.back != state.back)
			{
				trigger_mouse_button_event(event_handler, MouseButton::Back, next_state.back);

				change_detected = true;
			}
		}

		if (check_wheel)
		{
			if ((next_state.wheel_x != 0) || (next_state.wheel_y != 0))
			{
				trigger_mouse_scroll_event(event_handler, next_state.wheel_x, next_state.wheel_y);

				change_detected = true;
			}
		}

		// Notify the caller if a state change was detected.
		return change_detected;
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
			wheel_x, wheel_y
		);
	}
}