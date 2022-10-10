#include "mouse_profile.hpp"
#include "profile_metadata.hpp"

#include "input_profile_impl.hpp"

#include <util/json.hpp>

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
	}
}