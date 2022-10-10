#pragma once

//#include "types.hpp"

#include <util/json.hpp>
#include <util/format.hpp>

#include <magic_enum/magic_enum.hpp>

#include <string>
#include <string_view>
#include <optional>
#include <stdexcept>

namespace app::input
{
	// Currently only supports 4-directional Hat switch simulation.
	template <typename ButtonType>
	struct Hat
	{
		using Button = ButtonType;

		// URDL layout (meant to mimic SDL):
		Button up;
		Button right;
		Button down;
		Button left;

		// Name of this hat object. (optional)
		std::string name;

		Hat(const Hat&) = default;
		Hat(Hat&&) noexcept = default;

		Hat& operator=(const Hat&) noexcept = default;
		Hat& operator=(Hat&&) noexcept = default;

		//Hat() {}

		Hat(Button up, Button right, Button down, Button left, const std::string& name={})
			: up(up), right(right), down(down), left(left), name(name) {}

		Hat(const util::json& data, const std::string& name, bool load_name)
			: name(name)
		{
			load(data, load_name);
		}

		Hat(const util::json& data, const std::string& name={})
			: Hat(data, name, name.empty()) {}

		void load_button(const util::json& data, Button& button, std::string_view button_name)
		{
			// TODO: Optimize via heterogeneous lookup...?
			auto raw_value = data[std::string(button_name)].get<std::string>();
			auto binding   = magic_enum::enum_cast<ButtonType>(raw_value);

			if (!binding)
			{
				throw std::runtime_error(format("Unable to resolve binding for Hat button: \"{}\"", button_name));
			}

			button = *binding;
		}

		void load(const util::json& data, bool load_name=true)
		{
			load_button(data, up,    "up");
			load_button(data, right, "right");
			load_button(data, down,  "down");
			load_button(data, left,  "left");

			if (load_name)
			{
				name = data["name"].get<std::string>();
			}
		}
	};
}