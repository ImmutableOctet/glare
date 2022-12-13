#pragma once

#include "types.hpp"
#include "timer.hpp"
#include "entity_descriptor.hpp"
#include "entity_state.hpp"
#include "entity_state_rule.hpp"

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
			using CommandContent = MetaTypeDescriptor;
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
				std::string_view, // compared_value
				std::ptrdiff_t    // updated_offset
			>
			parse_trigger_condition(const std::string& trigger_condition, std::ptrdiff_t offset=0); // std::string_view

			static std::tuple
			<
				std::string_view, // type_name
				std::ptrdiff_t    // updated_offset
			>
			parse_event_type(const std::string& event_type, std::ptrdiff_t offset = 0); // std::string_view

			// TODO: Move to a different file/class:
			static std::optional<Timer::Duration> parse_time_duration(const std::string& time_expr); // std::string_view
			static std::optional<Timer::Duration> parse_time_duration(const util::json& time_data);

			// NOTE: The 'allow' arguments refer to construction of the underlying `MetaTypeDescriptor`.
			// To affect the component itself, use the `component_flags` argument.
			bool process_component
			(
				EntityDescriptor::TypeInfo& components_out,
				std::string_view component_name,
				const util::json* data=nullptr,

				std::optional<SmallSize> constructor_arg_count=std::nullopt,
				
				const MetaTypeDescriptorFlags& component_flags={},

				bool allow_entry_update=false,
				bool allow_new_entry=true,
				bool allow_default_entries=true
			);

			// Resolves a state from a (raw) path.
			// See also: `process_state`
			const EntityState* resolve_state
			(
				EntityDescriptor::StateCollection& states_out,
				std::string_view state_path_raw, // const std::string&
				const std::filesystem::path& base_path
			);

			// Returns a pointer to the processed state (stored in `states_out`) on success.
			const EntityState* process_state
			(
				EntityDescriptor::StateCollection& states_out,
				const util::json& data,
				std::string_view state_name,
				const std::filesystem::path& base_path
			);

			std::size_t process_state_list(EntityDescriptor::StateCollection& states_out, const util::json& data, const std::filesystem::path& base_path);

			// TODO: Optimize to avoid multiple calls to `parse_component_declaration`.
			template <typename Callback>
			inline std::size_t process_and_inspect_component_list
			(
				MetaDescription& components_out,
				const util::json& components,
				Callback&& callback,
				bool exit_on_false_value=true,

				const MetaTypeDescriptorFlags& shared_component_flags={},
				bool allow_new_entry=true,
				bool allow_default_entries=true
			)
			{
				process_component_list(components_out, components, shared_component_flags, allow_new_entry, allow_default_entries);

				std::size_t count = 0;

				auto inspect_component = [&count, &callback](std::string_view component_name)
				{
					if (callback(component_name))
					{
						count++;

						return true;
					}

					return false;
				};

				const auto container_type = components.type();

				switch (container_type)
				{
					case util::json::value_t::object:
					{
						for (const auto& proxy : components.items())
						{
							const auto& component_declaration = proxy.key();

							// Since component declarations can have additional symbols,
							// we'll need to extract only the 'name' substring.
							// 
							// TODO: Optimize by rolling `process_component_list` into this routine. (Parses declarations twice)
							auto component_decl_info = parse_component_declaration(component_declaration);
							const auto& component_name = std::get<0>(component_decl_info);

							const auto result = inspect_component(component_name);

							if (!result && exit_on_false_value)
							{
								break;
							}
						}

						break;
					}
					case util::json::value_t::array:
					{
						for (const auto& proxy : components.items())
						{
							const auto component_name = proxy.value().get<std::string>();

							const auto result = inspect_component(std::string_view(component_name));

							if (!result && exit_on_false_value)
							{
								break;
							}
						}

						break;
					}
					case util::json::value_t::string:
					{
						const auto component_name = components.get<std::string>();

						inspect_component(std::string_view(component_name));

						break;
					}
					default:
					{
						//print_warn("Unknown type identified in place of component data.");

						break;
					}
				}

				// See above for accumulation.
				return count;
			}

			std::size_t process_state_isolated_components(EntityState& state, const util::json& isolated);
			std::size_t process_state_local_copy_components(EntityState& state, const util::json& local_copy);
			std::size_t process_state_init_copy_components(EntityState& state, const util::json& init_copy);

			// NOTE: The `opt_states_out` and `opt_base_path` arguments are only used if `allow_inline_import` is enabled.
			std::size_t process_state_rules
			(
				EntityState& state,
				const util::json& rules,

				EntityDescriptor::StateCollection* opt_states_out = nullptr,
				const std::filesystem::path* opt_base_path = nullptr,
				bool allow_inline_import = true
			);

			// NOTE: Subroutine of `process_state_rules`.
			std::size_t process_state_rule
			(
				EntityState& state,
				MetaTypeID type_name_id,
				const util::json& content,
				
				std::optional<EventTriggerCondition> condition=std::nullopt,

				EntityDescriptor::StateCollection* opt_states_out=nullptr,
				const std::filesystem::path* opt_base_path=nullptr,
				bool allow_inline_import=true
			);

			// NOTE: Subroutine of `process_state_rules`.
			std::optional<EventTriggerSingleCondition> process_trigger_condition // std::optional<EventTriggerCondition>
			(
				const entt::meta_type& type,

				std::string_view member_name,
				std::string_view comparison_operator,
				std::string_view compared_value_raw,

				// Used for debugging purposes, etc.
				std::string_view trigger_condition_expr = {}
			);

			// NOTE: Subroutine of `process_state_rules`.
			std::size_t process_trigger_expression
			(
				EntityState& state,

				const std::string& trigger_condition_expr, // std::string_view
				const util::json& content,

				EntityDescriptor::StateCollection* opt_states_out=nullptr,
				const std::filesystem::path* opt_base_path=nullptr,
				bool allow_inline_import=true
			);

			const EntityState* process_state_inline_import
			(
				EntityDescriptor::StateCollection* states_out,
				const std::string& command, // std::string_view
				const std::filesystem::path* base_path,
				bool allow_inline_import=true
			);

			std::size_t process_component_list
			(
				EntityDescriptor::TypeInfo& components_out,
				const util::json& components,

				const MetaTypeDescriptorFlags& shared_component_flags={},

				bool allow_new_entry=true,
				bool allow_default_entries=true,
				bool forward_entry_update_condition_to_flags=false
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
						
						const auto base_path = archetype_path.parent_path();

						process_archetype(archetype, base_path, child_callback, resolve_external_modules, process_children);
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