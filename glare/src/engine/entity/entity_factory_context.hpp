#pragma once

#include <filesystem>
#include <utility>

namespace engine
{
	struct EntityFactoryContext
	{
		struct Paths
		{
			std::filesystem::path instance_path = {};

			// The local root directory for instances created with this factory.
			std::filesystem::path instance_directory = {};

			std::filesystem::path service_archetype_root_path = {};
			std::filesystem::path archetype_root_path = "archetypes"; // "engine/archetypes";
		};

		Paths paths;

		std::filesystem::path resolve_path(const std::filesystem::path& path, const std::filesystem::path& base_path) const;
		std::filesystem::path resolve_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const;

		inline EntityFactoryContext(Paths&& paths, const std::filesystem::path& base_path, bool resolve_instance_path=true)
			: paths(std::move(paths))
		{
			if (resolve_instance_path)
			{
				this->paths.instance_path = resolve_path(this->paths.instance_path, base_path);
			}
		}

		inline EntityFactoryContext(Paths&& paths, bool resolve_instance_path=true)
			: EntityFactoryContext(std::move(paths), {}, resolve_instance_path)
		{}

		EntityFactoryContext() = default;
	};
}