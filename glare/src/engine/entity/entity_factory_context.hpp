#pragma once

#include "types.hpp"

#include <filesystem>
#include <optional>
#include <utility>

namespace engine
{
	struct EntityFactoryContext
	{
		public:
			struct Paths
			{
				// The path used to build this factory.
				std::filesystem::path instance_path            = {};

				// The root directory for instances created with this factory. (Directory of `instance_path`)
				std::filesystem::path instance_directory          = {};

				// An optional shared directory to look for missing files.
				std::filesystem::path shared_directory            = {};

				// The root directory for the type of service this entity is constructed for.
				std::filesystem::path service_archetype_root_path = {};

				// The root directory for archetypes.
				std::filesystem::path archetype_root_path         = "engine/archetypes";
			};

			Paths paths;

			std::filesystem::path resolve_path(const std::filesystem::path& path, const std::filesystem::path& base_path) const;

			std::filesystem::path resolve_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const;
			std::filesystem::path resolve_entity_script_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const;

			std::filesystem::path resolve_cpp_script_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const;
			
			std::optional<std::pair<std::filesystem::path, PrecompiledScriptID>>
			resolve_cpp_script_reference_ex(const std::filesystem::path& path, const std::filesystem::path& base_path) const;

			inline EntityFactoryContext(Paths&& paths, const std::filesystem::path& base_path, bool resolve_instance_path=true)
				: paths(std::move(paths))
			{
				if (resolve_instance_path)
				{
					this->paths.instance_path = resolve_reference(this->paths.instance_path, base_path); // resolve_path(...);

					if (auto resolved_instance_directory = this->paths.instance_path.parent_path(); !resolved_instance_directory.empty())
					{
						this->paths.instance_directory = std::move(resolved_instance_directory);
					}
				}
			}

			inline EntityFactoryContext(Paths&& paths, bool resolve_instance_path=true)
				: EntityFactoryContext(std::move(paths), {}, resolve_instance_path)
			{}

			EntityFactoryContext() = default;

		private:
			template <typename ExistsFunction>
			std::filesystem::path resolve_path_impl(const std::filesystem::path& path, const std::filesystem::path& base_path, ExistsFunction&& exists_fn) const;

			std::filesystem::path resolve_path_impl(const std::filesystem::path& path, const std::filesystem::path& base_path) const;

			template <typename ExistsFunction>
			std::filesystem::path resolve_reference_impl(const std::filesystem::path& path, const std::filesystem::path& base_path, std::string_view file_extension, ExistsFunction&& exists_fn) const;

			std::filesystem::path resolve_reference_impl(const std::filesystem::path& path, const std::filesystem::path& base_path, std::string_view file_extension) const;
	};
}