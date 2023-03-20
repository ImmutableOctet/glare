#include "keyboard.hpp"
#include "keycodes.hpp"
#include "keyboard_events.hpp"

#include "input_profile_impl.hpp"
#include "input_device_impl.hpp"

#include <math/types.hpp>
#include <math/joyhat.hpp>

#include <entt/signal/dispatcher.hpp>

#include <sdl2/SDL_keyboard.h>

namespace app::input
{
	Keyboard::Keyboard(bool event_buttons)
		: event_buttons(event_buttons)
	{
		//peek(this->state);
	}

	Keyboard::State& Keyboard::poll_next_state(bool update_buttons, bool force_clear) const
	{
		if (force_clear)
		{
			next_state.clear_keys();
		}

		if (update_buttons)
		{
			int key_count = 0;

			const auto* key_data = SDL_GetKeyboardState(&key_count);

			next_state.set_keys
			(
				KeyboardState::KeyDataView
				(
					key_data,
					static_cast<KeyboardState::SizeType>(key_count)
					//(key_data + static_cast<KeyboardState::SizeType>(key_count))
				)
			);
		}

		return next_state;
	}

	const Keyboard::State& Keyboard::poll(entt::dispatcher* opt_event_handler)
	{
		bool manual_buttons = (!event_buttons);

		poll_next_state(manual_buttons, false);

		if (opt_event_handler)
		{
			handle_manual_event_detection
			(
				*opt_event_handler,
				manual_buttons
			);

			handle_hat_event_detection(*opt_event_handler, next_state);
		}

		return InputDevice<KeyboardState>::poll(opt_event_handler);
	}

	void Keyboard::handle_hat_event_detection(entt::dispatcher& event_handler, State& state, KeyboardDeviceIndex device_index) const
	{
		if (!device_profile)
		{
			return;
		}

		handle_hat_event_detection_impl<KeyboardMotion>
		(
			*device_profile, state,

			// get_button_state:
			[](const auto& state, const auto& button)
			{
				return state.get_key(button);
			},

			[this, &event_handler, &device_index](const auto& state, const auto& analog, const auto& input_direction)
			{
				event_handler.enqueue<OnKeyboardAnalogInput>
				(
					device_index,
					state,
					analog,
					input_direction
				);
			}
		);
	}

	void Keyboard::peek(Keyboard::State& state) const
	{
		state = next_state;
	}

	void Keyboard::flush()
	{
		//next_state.clear_keys();
	}

	bool Keyboard::process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler)
	{
		switch (e.type)
		{
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				if (event_buttons)
				{
					if (!e.key.keysym.scancode)
					{
						return false;
					}

					const auto button = static_cast<KeyboardButton>(e.key.keysym.scancode);
					const bool is_down = (e.key.state == SDL_PRESSED);

					next_state.set_key(button, is_down);

					if (opt_event_handler)
					{
						trigger_keyboard_button_event(*opt_event_handler, button, is_down);
					}

					return true;
				}

				break;
		}

		return false;
	}

	bool Keyboard::handle_manual_event_detection(entt::dispatcher& event_handler, bool check_buttons) const
	{
		bool change_detected = false;

		if (check_buttons)
		{
			// Previous state.
			const auto& prev_state = get_state();

			change_detected = next_state.observe
			(
				prev_state,
				
				[this, &event_handler](KeyboardButton button, bool current_value, bool previous_value) -> bool
				{
					if (current_value) // Button held.
					{
						// NOTE: Keyboard down/held events are continuous.
						trigger_keyboard_button_event(event_handler, button, true);
					}
					else if ((!current_value) && (previous_value)) // Button released.
					{
						trigger_keyboard_button_event(event_handler, button, false);
					}

					return (current_value != previous_value);
				}
			);
		}

		return change_detected;
	}

	void Keyboard::trigger_keyboard_button_event
	(
		entt::dispatcher& event_handler,
		KeyboardButton button,
		bool is_down,
		KeyboardDeviceIndex device_index
	) const
	{
		if (is_down)
		{
			event_handler.enqueue<OnKeyboardButtonDown>
			(
				device_index,
				next_state,
				button
			);
		}
		else
		{
			event_handler.enqueue<OnKeyboardButtonUp>
			(
				device_index,
				next_state,
				button
			);
		}
	}

	std::string Keyboard::get_device_name() const
	{
		return std::string(peek_device_name());
	}

	std::string_view Keyboard::peek_device_name() const
	{
		return "Keyboard";
	}

	const KeyboardProfile* Keyboard::get_profile() const
	{
		if (device_profile)
		{
			return &(device_profile.value());
		}

		return nullptr;
	}

	// TODO: Look into the ability to manage multiple mouse profiles.
	const KeyboardProfile* Keyboard::load_profile(const ProfileMetadata& profile_metadata)
	{
		return input_profile_impl::load_profile
		(
			profile_metadata,
			this->device_profile,
			"keyboards",
			this->peek_device_name()
		);
	}
}