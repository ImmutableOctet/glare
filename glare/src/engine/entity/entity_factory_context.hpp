#pragma once

#include <filesystem>

namespace engine
{
	struct EntityFactoryContext
	{
		struct
		{
			std::filesystem::path instance_path = {};

			// The local root directory for instances created with this factory.
			std::filesystem::path instance_directory = {};

			std::filesystem::path service_archetype_root_path = {};
			std::filesystem::path archetype_root_path = "archetypes"; // "engine/archetypes";
		} paths;

		std::filesystem::path resolve_path(const std::filesystem::path& path, const std::filesystem::path& base_path) const;
		std::filesystem::path resolve_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const;
	};
}