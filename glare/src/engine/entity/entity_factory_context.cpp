#include "entity_factory_context.hpp"

namespace engine
{
	std::filesystem::path EntityFactoryContext::resolve_path(const std::filesystem::path& path, const std::filesystem::path& base_path) const
	{
		if (!base_path.empty())
		{
			if (auto projected_path = (base_path / path); std::filesystem::exists(projected_path))
			{
				return projected_path;
			}
		}

		if (auto projected_path = (paths.instance_directory / path); std::filesystem::exists(projected_path))
		{
			return projected_path;
		}

		if (!paths.service_archetype_root_path.empty())
		{
			if (auto projected_path = (paths.service_archetype_root_path / path); std::filesystem::exists(projected_path))
			{
				return projected_path;
			}
		}

		if (auto projected_path = (paths.archetype_root_path / path); std::filesystem::exists(projected_path))
		{
			return projected_path;
		}

		if (std::filesystem::exists(path))
		{
			return path;
		}

		return {};
	}

	std::filesystem::path EntityFactoryContext::resolve_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const
	{
		auto module_path = resolve_path(path, base_path);

		if (module_path.empty())
		{
			auto direct_path = path; direct_path.replace_extension("json");

			module_path = resolve_path(direct_path, base_path);

			if (module_path.empty())
			{
				return {};
			}
		}
		else if (std::filesystem::is_directory(module_path))
		{
			const auto module_name = module_path.filename();

			return (module_path / module_name).replace_extension("json");
		}

		return module_path;
	}
}