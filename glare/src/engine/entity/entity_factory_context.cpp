#include "entity_factory_context.hpp"

#if (GLARE_SCRIPT_PRECOMPILED)
	#include <engine/meta/types.hpp>
	#include <engine/reflection/script.hpp>
#endif

namespace engine
{
	template <typename ExistsFunction>
	std::filesystem::path EntityFactoryContext::resolve_path_impl(const std::filesystem::path& path, const std::filesystem::path& base_path, ExistsFunction&& exists_fn) const
	{
		if (!base_path.empty())
		{
			if (auto projected_path = (base_path / path); exists_fn(projected_path))
			{
				return projected_path;
			}
		}

		if (!paths.instance_directory.empty())
		{
			if (auto projected_path = (paths.instance_directory / path); exists_fn(projected_path))
			{
				return projected_path;
			}
		}

		if (!paths.shared_directory.empty())
		{
			if (auto projected_path = (paths.shared_directory / path); exists_fn(projected_path))
			{
				return projected_path;
			}
		}

		if (!paths.service_archetype_root_path.empty())
		{
			if (auto projected_path = (paths.service_archetype_root_path / path); exists_fn(projected_path))
			{
				return projected_path;
			}
		}

		if (!paths.archetype_root_path.empty())
		{
			if (auto projected_path = (paths.archetype_root_path / path); exists_fn(projected_path))
			{
				return projected_path;
			}
		}

		if (exists_fn(path))
		{
			return path;
		}

		return {};
	}

	std::filesystem::path EntityFactoryContext::resolve_path_impl(const std::filesystem::path& path, const std::filesystem::path& base_path) const
	{
		return resolve_path_impl(path, base_path, [](const auto& projected_path) { return std::filesystem::exists(projected_path); });
	}

	std::filesystem::path EntityFactoryContext::resolve_path(const std::filesystem::path& path, const std::filesystem::path& base_path) const
	{
		return resolve_path_impl(path, base_path);
	}

	std::filesystem::path EntityFactoryContext::resolve_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const
	{
		return resolve_reference_impl(path, base_path, "json");
	}

	std::filesystem::path EntityFactoryContext::resolve_entity_script_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const
	{
		return resolve_reference_impl(path, base_path, "es");
	}

	std::optional<std::pair<std::filesystem::path, PrecompiledScriptID>>
	EntityFactoryContext::resolve_cpp_script_reference_ex(const std::filesystem::path& path, const std::filesystem::path& base_path) const
	{
		auto script_id_out = PrecompiledScriptID {};

#if GLARE_SCRIPT_PRECOMPILED
		const auto script_type = engine::resolve<engine::ScriptNamespace>();

		auto script_exists = [&script_type, &script_id_out](const auto& projected_path)
		{
			if (!script_type)
			{
				return false;
			}

			// Alternative implementation (slower):
			//const auto projected_path_normalized = projected_path.lexically_normal();

			// Assume the projected path is normalized, since the only situation where
			// it's not is on Windows, where we specifically account for that anyway.
			const auto& projected_path_normalized = projected_path;
			
			auto projected_path_as_string = projected_path_normalized.string();

#ifdef _WIN32
			// Replace backslashes with slashes on Windows platforms.
			util::replace(projected_path_as_string, "\\", "/");
#endif // _WIN32

			const auto script_id = engine::hash(projected_path_as_string);

			if (script_type.func(script_id))
			{
				script_id_out = script_id;

				return true;
			}

			return false;
		};
#else
		auto script_exists = [](const auto& projected_path) { return std::filesystem::exists(projected_path); };
#endif // GLARE_SCRIPT_PRECOMPILED

		if (auto cpp_path = resolve_reference_impl(path, base_path, "cpp", script_exists); !cpp_path.empty())
		{
			return { { cpp_path, script_id_out } };
		}

		if (auto cxx_path = resolve_reference_impl(path, base_path, "cxx", script_exists); !cxx_path.empty())
		{
			return { { cxx_path, script_id_out } };
		}

		if (auto C_path = resolve_reference_impl(path, base_path, "C", script_exists); !C_path.empty())
		{
			return { { C_path, script_id_out } };
		}

		return {};
	}

	std::filesystem::path EntityFactoryContext::resolve_cpp_script_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const
	{
		if (auto result = resolve_cpp_script_reference_ex(path, base_path))
		{
			return std::get<0>(*result);
		}

		return {};
	}

	template <typename ExistsFunction>
	std::filesystem::path EntityFactoryContext::resolve_reference_impl(const std::filesystem::path& path, const std::filesystem::path& base_path, std::string_view file_extension, ExistsFunction&& exists_fn) const
	{
		// TODO: Look into switching lookup order here.
		// (Exact file extensions should resolve a lot faster than this)
		auto module_path = resolve_path_impl(path, base_path, exists_fn);

		if (module_path.empty())
		{
			auto direct_path = path; direct_path.replace_extension(file_extension);

			module_path = resolve_path_impl(direct_path, base_path, exists_fn);

			if (module_path.empty())
			{
				return {};
			}
		}
		else if (std::filesystem::is_directory(module_path))
		{
			const auto module_name = module_path.filename();

			return (module_path / module_name).replace_extension(file_extension);
		}

		return module_path;
	}

	std::filesystem::path EntityFactoryContext::resolve_reference_impl(const std::filesystem::path& path, const std::filesystem::path& base_path, std::string_view file_extension) const
	{
		return resolve_reference_impl(path, base_path, file_extension, [](const auto& projected_path) { return std::filesystem::exists(projected_path); });
	}
}