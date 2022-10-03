#include "buttons.hpp"

#include <magic_enum/magic_enum.hpp>

namespace engine
{
	void generate_button_map(EngineButtonMap& buttons)
	{
		constexpr auto entries = magic_enum::enum_entries<Button>();

		for (const auto& enum_entry : entries)
		{
			buttons[std::string(enum_entry.second)] = static_cast<EngineButtonsRaw>(enum_entry.first);
		}
	}
}