#pragma once

#include "types.hpp"
#include "entity_descriptor.hpp"
#include "entity_state_rule.hpp"
#include "timer.hpp"

#include <util/json.hpp>
#include <util/algorithm.hpp>

#include <filesystem>
#include <string>
#include <tuple>
#include <optional>
#include <string_view>

namespace engine
{
	//class World;
	class ResourceManager;

	//template <typename ServiceType>
	struct EntityConstructionContext
	{
		//using ServiceType = World;
		//ServiceType& service;

		Registry& registry;
		ResourceManager& resource_manager;

		//std::filesystem::path instance_path;

		Entity parent = null;

		// If this field is left as `null`, a factory will
		// generate an appropriate Entity instance.
		Entity opt_entity_out = null;
	};

	// TODO: Look into moving `registry` and `resource_manager` out of this type.
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
	};

	class EntityFactory : protected EntityFactoryContext
	{
		public:
			using FactoryKey = std::string;
			using SmallSize = MetaTypeDescriptor::SmallSize;

			//using json = util::json;
		protected:
			EntityDescriptor descriptor;

			std::optional<EntityStateIndex> default_state_index = std::nullopt;
		public:
			std::filesystem::path resolve_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const;

			template <typename ChildFactoryCallback>
			inline EntityFactory(const EntityFactoryContext& factory_context, ChildFactoryCallback&& child_callback, bool resolve_external_modules=true, bool process_children=true)
				: EntityFactoryContext(factory_context)
			{
				// TODO: Optimize.
				auto instance = util::load_json(paths.instance_path);

				process_archetype(instance, paths.instance_directory, child_callback, resolve_external_modules, process_children);
			}

			inline EntityFactory(const EntityFactoryContext& factory_context, bool resolve_external_modules=true)
				: EntityFactoryContext(factory_context)
			{
				// TODO: Optimize.
				auto instance = util::load_json(paths.instance_path);

				process_archetype(instance, paths.instance_directory, resolve_external_modules);
			}

			EntityFactory(const EntityFactory&) = default;
			EntityFactory(EntityFactory&&) noexcept = default;

			EntityFactory& operator=(const EntityFactory&) = default;
			EntityFactory& operator=(EntityFactory&&) noexcept = default;

			Entity create(const EntityConstructionContext& context) const;

			inline Entity operator()(const EntityConstructionContext& context) const
			{
				return create(context);
			}

			inline const EntityDescriptor& get_descriptor() const
			{
				return descriptor;
			}

			inline const std::filesystem::path& get_instance_path() const
			{
				return paths.instance_path;
			}
		protected:
			static std::tuple<std::string_view, bool, std::optional<SmallSize>>
			parse_component_declaration(const std::string& component_declaration); // std::string_view

			static std::tuple
			<
				std::string_view, // type_name
				std::string_view, // member_name
				std::string_view, // comparison_operator
				std::string_view  // compared_value
			>
			parse_trigger_condition(const std::string& trigger_condition); // std::string_view

			static std::tuple
			<
				std::string_view, // command_name
				std::string_view, // command_content
				bool              // is_string_content
			> parse_single_argument_command(const std::string& command); // std::string_view

			// TODO: Move to a different file/class:
			static std::optional<Timer::Duration> parse_time_duration(const std::string& time_expr); // std::string_view
			static std::optional<Timer::Duration> parse_time_duration(const util::json& time_data);

			// TODO: Move to a different file/class.
			static EntityStateTransitionRule::TargetType process_rule_target(const util::json& target_data);

			bool process_component
			(
				EntityDescriptor::TypeInfo& components_out,
				std::string_view component_name,
				const util::json* data=nullptr,
				bool allow_inplace_changes=false,
				std::optional<SmallSize> constructor_arg_count=std::nullopt
			);

			bool process_state
			(
				EntityDescriptor::StateCollection& states_out,
				const util::json& data,
				std::string_view state_name
			);

			std::size_t process_state_isolated_components(EntityState& state, const util::json& isolated);

			std::size_t process_state_rules(EntityState& state, const util::json& rules);

			void process_component_list
			(
				EntityDescriptor::TypeInfo& components_out,
				const util::json& components
			);

			void process_archetype(const util::json& data, const std::filesystem::path& base_path, bool resolve_external_modules=true);

			template <typename ChildFactoryCallback>
			inline void process_archetype(const util::json& data, const std::filesystem::path& base_path, ChildFactoryCallback&& child_callback, bool resolve_external_modules=true, bool process_children=true)
			{
				// Override external module resolution.
				if (resolve_external_modules)
				{
					resolve_archetypes(data, base_path, child_callback, true, process_children);
				}

				// Execute main overload without external modules.
				process_archetype(data, base_path, false);

				if (process_children)
				{
					if (auto children = data.find("children"); children != data.end())
					{
						util::json_for_each<util::json::value_t::string>
						(
							*children,
							[this, &base_path, &child_callback](const auto& value)
							{
								const auto child_path_raw = std::filesystem::path(value.get<std::string>());
								const auto child_path = resolve_reference(child_path_raw, base_path);

								child_callback
								(
									*this,

									EntityFactoryContext
									{
										.paths =
										{
											.instance_path = child_path,
											.instance_directory = child_path.parent_path(),
											.service_archetype_root_path = this->paths.service_archetype_root_path,
											.archetype_root_path = this->paths.archetype_root_path
										}
									}
								);
							}
						);
					}
				}
			}

			// Resolves the contents of the `archetypes` field pointed to by `instance`.
			// 
			// If an `archetypes` field is not present in `instance`,
			// or if its contents could not be read, this will return false.
			// 
			// See also: `process_archetype`.
			template <typename ChildFactoryCallback>
			inline bool resolve_archetypes(const util::json& instance, const std::filesystem::path& base_path, ChildFactoryCallback&& child_callback, bool resolve_external_modules=true, bool process_children=true)
			{
				auto archetypes = util::find_any(instance, "archetypes", "import", "modules"); // instance.find("archetypes");

				if (archetypes == instance.end())
				{
					return false;
				}

				auto elements_processed = util::json_for_each<util::json::value_t::string>
				(
					*archetypes,

					[this, &base_path, &child_callback, resolve_external_modules, process_children](const auto& value)
					{
						const auto archetype_path_raw = std::filesystem::path(value.get<std::string>());
						const auto archetype_path = resolve_reference(archetype_path_raw, base_path);

						if (archetype_path.empty())
						{
							return;
						}

						// TODO: Optimize.
						auto archetype = util::load_json(archetype_path);

						process_archetype(archetype, archetype_path.parent_path(), child_callback, resolve_external_modules, process_children);
					}
				);

				return (elements_processed > 0);
			}

			inline bool resolve_archetypes(const util::json& instance, const std::filesystem::path& base_path, bool resolve_external_modules=true)
			{
				return resolve_archetypes
				(
					instance, base_path,
					[](const auto& parent_factory, const auto& child_ctx) {},
					resolve_external_modules,
					false
				);
			}
	};
}