#include "keyboard_profile.hpp"
#include "profile_metadata.hpp"

#include "input_profile_impl.hpp"

#include <util/json.hpp>

#include <optional>

namespace app::input
{
	KeyboardProfile::KeyboardProfile(const ProfileMetadata& profile_metadata, const util::json& json)
		: KeyboardProfile()
	{
		load(profile_metadata, json);
	}

	void KeyboardProfile::load(const ProfileMetadata& profile_metadata, const util::json& json)
	{
		input_profile_impl::profile_load_basics(*this, profile_metadata, json);
	}
}