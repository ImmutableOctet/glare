#pragma once

#include "types.hpp"

#include "entity_factory_context.hpp"
#include "entity_descriptor.hpp"
#include "serial.hpp"

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

	class EntityState;

	struct EntityStateRule;
	struct EntityThreadDescription;
	struct CommandParsingContext;

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

	class EntityFactory : protected EntityFactoryContext
	{
		public:
			using FactoryKey = std::string;
			using CommandContent = MetaTypeDescriptor; // EntityStateCommandAction::CommandContent;
			using SmallSize = MetaTypeDescriptor::SmallSize;

			//using json = util::json;
		protected:
			EntityDescriptor descriptor;

			std::optional<EntityStateIndex> default_state_index = std::nullopt;
		public:
			template <typename ChildFactoryCallback>
			inline EntityFactory
			(
				const EntityFactoryContext& factory_context,
				ChildFactoryCallback&& child_callback,
				const CommandParsingContext* opt_command_context=nullptr,
				bool resolve_external_modules=true, bool process_children=true
			)
				: EntityFactoryContext(factory_context)
			{
				// TODO: Optimize.
				auto instance = util::load_json(paths.instance_path);

				process_archetype(instance, paths.instance_directory, child_callback, opt_command_context, resolve_external_modules, process_children);
			}

			inline EntityFactory
			(
				const EntityFactoryContext& factory_context,
				const CommandParsingContext* opt_command_context=nullptr,
				bool resolve_external_modules=true
			)
				: EntityFactoryContext(factory_context)
			{
				// TODO: Optimize.
				auto instance = util::load_json(paths.instance_path);

				process_archetype(instance, paths.instance_directory, opt_command_context, resolve_external_modules);
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
			CommandContent& generate_empty_command(const MetaType& command_type, Entity source=null, Entity target=null);

			// Utility function for loading JSON data.
			std::tuple
			<
				std::filesystem::path, // state_base_path
				std::filesystem::path, // state_path
				util::json             // state_data
			>
			load_state_data(std::string_view state_path_raw, const std::filesystem::path& base_path);

			static std::string get_embedded_name(const util::json& data);
			static std::string default_state_name_from_path(const std::filesystem::path& state_path);
			static std::string resolve_state_name(const util::json& state_data, const std::filesystem::path& state_path);

			// This overload resolves and processes a state from a raw path.
			// 
			// If successful, the value returned by this function
			// is a non-owning pointer to the processed state. (Stored in `states_out`)
			const EntityState* process_state
			(
				EntityDescriptor::StateCollection& states_out,
				std::string_view state_path_raw, // const std::string&
				const std::filesystem::path& base_path,
				const CommandParsingContext* opt_command_context=nullptr
			);

			// This overload acts as a utility function that automatically handles
			// allocation and storage of an `EntityState` object. To process using an
			// existing instance, please see the overload taking in an `EntityState` object.
			// 
			// This function returns a non-owning pointer to the processed state on success.
			// (Lifetime is owned by `states_out`)
			//
			// NOTE: This overload automatically handles name assignment. (Based on `state_name`)
			const EntityState* process_state
			(
				EntityDescriptor::StateCollection& states_out,
				const util::json& data,
				std::string_view state_name,
				const std::filesystem::path& base_path,
				const CommandParsingContext* opt_command_context=nullptr
			);

			// The `states_out` argument is used only for import resolution.
			// Please use the `state` argument for specifying an existing `EntityState` instance.
			// 
			// The return value of this function indicates if processing was successful.
			//
			// NOTE: This overload does not handle name resolution and assignment. (see `resolve_state_name`)
			bool process_state
			(
				EntityDescriptor::StateCollection& states_out,
				EntityState& state,
				const util::json& data,
				const std::filesystem::path& base_path,
				const CommandParsingContext* opt_command_context=nullptr
			);

			std::size_t process_state_list
			(
				EntityDescriptor::StateCollection& states_out,
				const util::json& data,
				const std::filesystem::path& base_path,
				const CommandParsingContext* opt_command_context=nullptr
			);

			// Merges one or more states defined in `data` with `state`.
			// 
			// NOTE: `states_out` is to be provided by the caller to handle
			// any imports encountered while processing `state`.
			// 
			// Further ownership of `state` is the caller's responsibility.
			//
			// The return value of this function indicates
			// how many states were successfully merged.
			std::size_t merge_state_list
			(
				EntityDescriptor::StateCollection& states_out,
				EntityState& state,

				const util::json& data,
				const std::filesystem::path& base_path,
				const CommandParsingContext* opt_command_context=nullptr
			);

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
			std::size_t process_state_rule_list
			(
				EntityState& state,
				const util::json& rules,

				EntityDescriptor::StateCollection* opt_states_out = nullptr,
				const std::filesystem::path* opt_base_path = nullptr,
				const CommandParsingContext* opt_command_context=nullptr,
				bool allow_inline_import = true
			);

			// NOTE: Subroutine of `process_state_rule_list`.
			std::size_t process_state_rule
			(
				EntityState& state,
				MetaTypeID type_name_id,
				const util::json& content,
				
				std::optional<EventTriggerCondition> condition=std::nullopt,

				EntityDescriptor::StateCollection* opt_states_out=nullptr,
				const std::filesystem::path* opt_base_path=nullptr,
				const CommandParsingContext* opt_command_context=nullptr,

				bool allow_inline_import=true
			);

			// NOTE: Subroutine of `process_state_rule_list`.
			std::size_t process_trigger_expression
			(
				EntityState& state,

				const std::string& trigger_condition_expr, // std::string_view
				const util::json& content,

				EntityDescriptor::StateCollection* opt_states_out=nullptr,
				const std::filesystem::path* opt_base_path=nullptr,
				const CommandParsingContext* opt_command_context=nullptr,
				bool allow_inline_import=true
			);

			//std::size_t
			std::tuple
			<
				EntityThreadIndex, // initial_thread_index
				EntityThreadCount  // processed_count
			>
			process_thread_list
			(
				const util::json& content,
				const std::filesystem::path* opt_base_path=nullptr,
				const CommandParsingContext* opt_command_context=nullptr
			);

			std::optional<std::tuple<EntityThreadIndex, const EntityThreadDescription*>>
			process_thread
			(
				const util::json& content,
				std::string_view opt_thread_name={},
				const std::filesystem::path* opt_base_path=nullptr,
				const CommandParsingContext* opt_command_context=nullptr
			);

			std::size_t process_thread
			(
				EntityThreadDescription& thread,
				const util::json& content,
				std::string_view opt_thread_name={},
				const std::filesystem::path* opt_base_path=nullptr,
				const CommandParsingContext* opt_command_context=nullptr
			);

			const EntityState* process_state_inline_import
			(
				EntityDescriptor::StateCollection* states_out,
				const std::string& command, // std::string_view
				const std::filesystem::path* base_path,
				const CommandParsingContext* opt_command_context=nullptr,
				bool allow_inline_import=true
			);

			void process_archetype
			(
				const util::json& data,
				const std::filesystem::path& base_path,
				const CommandParsingContext* opt_command_context=nullptr,
				bool resolve_external_modules=true
			);

			template <typename ChildFactoryCallback>
			inline void process_archetype
			(
				const util::json& data,
				const std::filesystem::path& base_path,
				ChildFactoryCallback&& child_callback,
				const CommandParsingContext* opt_command_context=nullptr,
				bool resolve_external_modules=true,
				bool process_children=true
			)
			{
				// Override external module resolution.
				if (resolve_external_modules)
				{
					resolve_archetypes(data, base_path, child_callback, opt_command_context, true, process_children);
				}

				// Execute main overload without external modules.
				process_archetype(data, base_path, opt_command_context, false);

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
			inline bool resolve_archetypes
			(
				const util::json& instance,
				const std::filesystem::path& base_path,
				ChildFactoryCallback&& child_callback,
				const CommandParsingContext* opt_command_context=nullptr,
				bool resolve_external_modules=true, bool process_children=true
			)
			{
				auto archetypes = util::find_any(instance, "archetypes", "import", "modules"); // instance.find("archetypes");

				if (archetypes == instance.end())
				{
					return false;
				}

				auto elements_processed = util::json_for_each<util::json::value_t::string>
				(
					*archetypes,

					[this, &base_path, &child_callback, opt_command_context, resolve_external_modules, process_children](const auto& value)
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

						process_archetype(archetype, base_path, child_callback, opt_command_context, resolve_external_modules, process_children);
					}
				);

				return (elements_processed > 0);
			}

			inline bool resolve_archetypes
			(
				const util::json& instance,
				const std::filesystem::path& base_path,
				const CommandParsingContext* opt_command_context=nullptr,
				bool resolve_external_modules=true
			)
			{
				return resolve_archetypes
				(
					instance, base_path,
					[](const auto& parent_factory, const auto& child_ctx) {},
					opt_command_context,
					resolve_external_modules,
					false
				);
			}
	};
}