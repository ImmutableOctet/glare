#include "mouse_profile.hpp"
#include "profile_metadata.hpp"

#include "input_profile_impl.hpp"

#include <util/json.hpp>
#include <tuple>

namespace app::input
{
	MouseProfile::MouseProfile(const ProfileMetadata& profile_metadata, const util::json& json)
		: MouseProfile()
	{
		load(profile_metadata, json);
	}

	void MouseProfile::load(const ProfileMetadata& profile_metadata, const util::json& json)
	{
		input_profile_impl::profile_load_basics(*this, profile_metadata, json);

		sensitivity = util::get_value(json, "sensitivity", sensitivity);

		invert_x = util::get_value(json, "invert_x", invert_x);
		invert_y = util::get_value(json, "invert_y", invert_y);
	}

	MouseAnalogInput MouseProfile::get_analog_value(MouseAnalogInput input) const
	{
		auto x = std::get<0>(input);
		auto y = std::get<1>(input);

		if (invert_x)
		{
			x = -x;
		}

		if (invert_y)
		{
			y = -y;
		}

		return { x, y };
	}
}