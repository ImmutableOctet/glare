#pragma once

#include "types.hpp"

//#include <functional>
//#include <tuple>
#include <filesystem>

namespace app::input
{
	// Metadata used to load a device profile.
	struct ProfileMetadata
	{
		const std::filesystem::path& path; // std::filesystem::path

		// Mapping of engine-defined button names to their corresponding engine-defined values.
		// This is useful to determine how to map names to device-specific button indices, etc.
		const EngineButtonMap& buttons;

		// Mapping of engine-defined analog names to their corresponding engine-defined values.
		// This is useful to determine how to map names to device-specific equivalents.
		const EngineAnalogMap& analogs;

		// Output from a load operation; device-to-player mappings.
		PlayerDeviceMap& player_mappings_out;
	};
}