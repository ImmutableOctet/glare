#include "analogs.hpp"

#include <math/math.hpp>

#include <util/magic_enum.hpp>

namespace engine
{
	InputAnalogStates::DirectionVector InputAnalogStates::get_analog(Analog analog) const
	{
		const auto  offset        = static_cast<std::ptrdiff_t>(analog);
		const auto* base_address  = reinterpret_cast<const unsigned char*>(this);
		const auto* analog_values = reinterpret_cast<const DirectionVector*>(base_address + offset);

		return *analog_values;
	}

	void InputAnalogStates::set_analog(Analog analog, const DirectionVector& value)
	{
		auto  offset        = static_cast<std::ptrdiff_t>(analog);
		auto* base_address  = reinterpret_cast<unsigned char*>(this);
		auto* analog_values = reinterpret_cast<DirectionVector*>(base_address + offset);

		*analog_values = value;
	}

	float InputAnalogStates::angle_of(const DirectionVector& analog) const
	{
		return math::direction_to_angle(analog);
	}

	float InputAnalogStates::angle_of(Analog analog) const
	{
		return angle_of(get_analog(analog));
	}

	float InputAnalogStates::movement_angle() const
	{
		return angle_of(movement);
	}

	float InputAnalogStates::camera_angle() const
	{
		return angle_of(camera);
	}

	float InputAnalogStates::menu_select_angle() const
	{
		return angle_of(menu_select);
	}

	float InputAnalogStates::orientation_angle() const
	{
		return angle_of(orientation);
	}

	void generate_analog_map(EngineAnalogMap& analogs)
	{
		constexpr auto entries = magic_enum::enum_entries<Analog>();

		for (const auto& enum_entry : entries)
		{
			analogs[std::string(enum_entry.second)] = static_cast<AnalogsRaw>(enum_entry.first);
		}
	}
}