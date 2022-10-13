#pragma once

//#include "types.hpp"

#include <util/json.hpp>
#include <util/format.hpp>

#include <util/magic_enum.hpp>

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
		std::optional<Button> up;
		std::optional<Button> right;
		std::optional<Button> down;
		std::optional<Button> left;

		// Name of this hat object. (optional)
		std::string name;

		bool is_active : 1 = false;

		Hat(const Hat&) = default;
		Hat(Hat&&) noexcept = default;

		Hat& operator=(const Hat&) noexcept = default;
		Hat& operator=(Hat&&) noexcept = default;

		//Hat() {}

		Hat(std::optional<Button> up, std::optional<Button> right, std::optional<Button> down, std::optional<Button> left, const std::string& name={})
			: up(up), right(right), down(down), left(left), name(name) {}

		Hat(const util::json& data, const std::string& name, bool load_name)
			: name(name)
		{
			load(data, load_name);
		}

		Hat(const util::json& data, const std::string& name={})
			: Hat(data, name, name.empty()) {}

		void load_button(const util::json& data, std::optional<Button>& button, std::string_view button_name)
		{
			const auto button_data_it = data.find(std::string(button_name));

			if (button_data_it == data.end())
			{
				return;
			}

			// TODO: Optimize via heterogeneous lookup...?
			const auto& button_data = *button_data_it;

			if (button_data.empty())
			{
				return;
			}

			auto raw_value = button_data.get<std::string>();

			if (raw_value.empty())
			{
				return;
			}

			auto binding = magic_enum::enum_cast<ButtonType>(raw_value);

			if (!binding)
			{
				throw std::runtime_error(format("Unable to resolve binding for Hat button: \"{}\"", button_name));
			}

			button = *binding; // binding;
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

		void set_active(bool value)
		{
			is_active = value;
		}

		bool get_active() const
		{
			return is_active;
		}
	};
}