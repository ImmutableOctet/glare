#include "keyboard_state.hpp"

#include <utility>

//#include <sdl2/SDL_keyboard.h>

namespace app::input
{
	KeyboardState::KeyboardState() {}

	KeyboardState::KeyboardState(Keys&& keys)
		: keys(std::move(keys))
	{}

	KeyboardState::KeyboardState(const Keys& keys)
		: KeyboardState(Keys(keys)) {}

	KeyboardState::KeyboardState(KeyDataView raw_key_data)
	{
		set_keys(raw_key_data);
	}

	KeyboardState& KeyboardState::operator=(KeyDataView raw_key_data)
	{
		set_keys(raw_key_data);

		return *this;
	}

	bool KeyboardState::operator==(const KeyboardState& state) const
	{
		for (SizeType i = 0; i < MAX_KEYS; i++)
		{
			if (keys[i] != state.keys[i])
			{
				return false;
			}
		}

		return true;
	}

	KeyboardState::SizeType KeyboardState::set_keys(KeyDataView raw_key_data, bool force_clear)
	{
		if (force_clear)
		{
			clear_keys();
		}

		const auto key_count = raw_key_data.size();

		for (SizeType i = 0; i < key_count; i++)
		{
			keys[i] = (raw_key_data[i] > 0);
		}

		return key_count;
	}

	void KeyboardState::clear_keys()
	{
		//keys = {};
		keys.reset();
	}

	void KeyboardState::set_key(KeyboardButton key, bool value)
	{
		const auto scan_code_idx = static_cast<SizeType>(key); // KeyboardButtonID

		keys[scan_code_idx] = value;
	}

	bool KeyboardState::get_key(KeyboardButton key) const
	{
		if (!has_keys()) // (scan_code < 0) || (scan_code >= keys.size())
		{
			return false;
		}

		const auto scan_code_idx = static_cast<SizeType>(key); // KeyboardButtonID

		return get_key(scan_code_idx);
	}

	bool KeyboardState::get_key(SizeType scan_code_idx) const
	{
		return (keys[scan_code_idx]);
	}

	bool KeyboardState::has_keys() const
	{
		return keys.any();
	}
}