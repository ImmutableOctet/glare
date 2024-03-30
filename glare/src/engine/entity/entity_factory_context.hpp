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
			using PathType = std::filesystem::path;

			// Normalizes a path so that forward slashes are used.
			static PathType& normalize_path_inplace(PathType& path);

			// Normalizes a path so that forward slashes are used.
			static PathType normalize_path(const PathType& path);

			struct Paths
			{
				// The path used to build this factory.
				PathType instance_path               = {};

				// The root directory for instances created with this factory. (Directory of `instance_path`)
				PathType instance_directory          = {};

				// An optional shared directory to look for missing files.
				PathType shared_directory            = {};

				// The root directory for the type of service this entity is constructed for.
				PathType service_archetype_root_path = {};

				// The root directory for archetypes.
				PathType archetype_root_path         = "engine/archetypes";

				void normalize()
				{
					normalize_path_inplace(instance_path);
					normalize_path_inplace(instance_directory);
					normalize_path_inplace(shared_directory);
					normalize_path_inplace(service_archetype_root_path);
					normalize_path_inplace(archetype_root_path);
				}
			};

			PathType resolve_path(const PathType& path, const PathType& base_path) const;

			PathType resolve_reference(const PathType& path, const PathType& base_path) const;
			PathType resolve_entity_script_reference(const PathType& path, const PathType& base_path) const;

			PathType resolve_cpp_script_reference(const PathType& path, const PathType& base_path) const;
			
			std::optional<std::pair<PathType, PrecompiledScriptID>>
			resolve_cpp_script_reference_ex(const PathType& path, const PathType& base_path) const;

			EntityFactoryContext() = default;

			inline EntityFactoryContext(Paths&& paths, const PathType& base_path, bool resolve_instance_path=true, bool normalize_paths=true)
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

				if (normalize_paths)
				{
					this->paths.normalize();
				}
			}

			inline EntityFactoryContext(Paths&& paths, bool resolve_instance_path=true, bool normalize_paths=true)
				: EntityFactoryContext(std::move(paths), {}, resolve_instance_path, normalize_paths)
			{}

			Paths paths;
		private:
			template <typename ExistsFunction>
			PathType resolve_path_impl(const PathType& path, const PathType& base_path, ExistsFunction&& exists_fn) const;

			PathType resolve_path_impl(const PathType& path, const PathType& base_path) const;

			template <typename ExistsFunction>
			PathType resolve_reference_impl(const PathType& path, const PathType& base_path, std::string_view file_extension, ExistsFunction&& exists_fn) const;

			PathType resolve_reference_impl(const PathType& path, const PathType& base_path, std::string_view file_extension) const;
	};
}