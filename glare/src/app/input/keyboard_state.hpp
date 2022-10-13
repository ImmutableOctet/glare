#pragma once

#include <types.hpp>

#include "keyboard_buttons.hpp"

#include <util/memory.hpp>

#include <span>
#include <array>
#include <bitset>
#include <optional>

namespace app::input
{
	struct KeyboardState
	{
		public:
			using Button = KeyboardButton;
			using RawButtonValue = KeyboardButtonValue;
			using SizeType = std::size_t;

			static constexpr auto MAX_KEYS = static_cast<SizeType>(Button::MAX);

			using Keys = std::bitset<MAX_KEYS>;
			using KeyDataView = std::span<const RawButtonValue, MAX_KEYS>;

			KeyboardState();
			KeyboardState(Keys&&);
			KeyboardState(const Keys&);
			KeyboardState(KeyDataView raw_key_data);

			KeyboardState(const KeyboardState&) = default;
			KeyboardState(KeyboardState&&) noexcept = default;

			KeyboardState& operator=(const KeyboardState&) = default;
			KeyboardState& operator=(KeyboardState&&) = default;

			KeyboardState& operator=(KeyDataView raw_key_data);

			bool operator==(const KeyboardState& state) const;

			inline bool operator!=(const KeyboardState& state) const
			{
				return !(operator==(state));
			}

			// Detect changes between this state and the `prev_state` specified.
			// 
			// `callback` is a callable taking a `KeyboardButton` and a boolean value, indicating this state's value.
			// Unlike `observe`, there is no need to take in two boolean values, as `previous_value` is already known
			// to be the logical opposite of the value provided. Additionally, no return value is necessary.
			template <typename Callback>
			inline bool detect_changes(const KeyboardState& prev_state, Callback&& callback) const
			{
				return observe(prev_state, [&callback](KeyboardButton button, bool current_value, bool previous_value) -> bool
				{
					if (current_value != previous_value)
					{
						callback(button, current_value);

						return true;
					}

					return false;
				});
			}

			// Observe the current state's keys alongside the previous state's keys.
			// 
			// `callback` is a callable taking in a `KeyboardButton` and two boolean values
			// indicating this state's value and the previous state's value.
			// 
			// Unlike `detect_changes`, this `callback` must return a value indicating
			// whether the caller thinks a change was detected.
			template <typename Callback>
			inline bool observe(const KeyboardState& prev_state, Callback&& callback) const
			{
				bool change_detected = false;

				for (SizeType i = 0; i < MAX_KEYS; i++) // KeyboardButtonID
				{
					const auto current_value  = keys[i];
					const auto previous_value = prev_state.keys[i];

					const auto button = static_cast<KeyboardButton>(i);

					if (callback(button, current_value, previous_value))
					{
						change_detected = true;
					}
				}

				return change_detected;
			}

			// Sets the internal key states using `raw_key_data` (unencoded array/span of values).
			SizeType set_keys(KeyDataView raw_key_data, bool force_clear=true);

			// Clears the state of all keys.
			void clear_keys();

			// Sets the `key` specified as being held/pressed.
			void set_key(KeyboardButton key, bool value);

			// Checks if the `key` specified is currently being held/pressed.
			bool get_key(KeyboardButton key) const;

			inline bool get_key(std::optional<KeyboardButton> opt_key) const
			{
				if (!opt_key)
				{
					return false;
				}

				return get_key(*opt_key);
			}

			// Checks if the `key` (raw value) specified is currently being held/pressed.
			bool get_key(SizeType scan_code_idx) const;

			// Shorthand for `set_key` with `value` of false.
			inline void clear_key(KeyboardButton key)
			{
				set_key(key, false);
			}

			// Indicates whether at least one key has been pressed/held.
			bool has_keys() const;

			// TODO: Determine if `keys` should be publicly accessible.
			Keys keys;
	};
}