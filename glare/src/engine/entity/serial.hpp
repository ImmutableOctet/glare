#pragma once

// TODO: Replace most `EntityDescriptor` parameters with an abstracted shared-storage type.

#include "parse.hpp"

#include "entity_factory_context.hpp"
#include "entity_target.hpp"
#include "entity_state_action.hpp"
#include "entity_instruction.hpp"

//#include "entity_descriptor.hpp"

#include <engine/timer.hpp>

#include <engine/meta/serial.hpp>
#include <engine/meta/meta_parsing_context.hpp>

#include <engine/world/animation/types.hpp>

#include <util/json.hpp>
#include <util/parse.hpp>
#include <util/algorithm.hpp>

#include <optional>
#include <utility>
#include <string>
#include <string_view>
#include <tuple>
//#include <filesystem>

namespace engine
{
	class SharedStorageInterface;
	class EntityDescriptor;
	class EntityState;
	class MetaParsingContext;

	struct EntityStateCollection;
	struct MetaTypeDescriptorFlags;
	struct MetaDescription;
	struct EntityStateRule;
	struct EntityThreadDescription;

	class AnimationRepository;
	struct AnimationSlice;

	// Offset used to account for the `source` and `target` arguments of a `Command` type.
	inline constexpr std::size_t command_arg_offset = 2;

	std::tuple
	<
		std::string_view, // type_name
		std::size_t       // updated_offset
	>
	parse_event_type(const std::string& event_type, std::size_t offset=0); // std::string_view

	std::tuple
	<
		std::string_view,           // component_name
		bool,                       // allow_entry_update (In-place changes)
		std::optional<std::uint8_t> // std::optional<MetaTypeDescriptor::SmallSize>
	>
	parse_component_declaration(const std::string& component_declaration, bool default_allow_entry_update=false); // std::string_view

	std::tuple
	<
		std::string_view,                       // instruction
		std::optional<EntityThreadInstruction>, // thread_details
		bool                                    // thread_accessor_used
	>
	parse_instruction_header
	(
		std::string_view instruction_raw,
		const EntityDescriptor* opt_descriptor=nullptr
	);

	std::tuple
	<
		std::optional<EntityThreadInstruction>, // thread_details
		bool                                    // accessor_used
	>
	parse_thread_details
	(
		std::string_view combined_expr,

		const EntityDescriptor* opt_descriptor=nullptr
	);

	std::tuple
	<
		std::optional<EntityThreadInstruction>, // thread_details
		bool                                    // accessor_used
	>
	parse_thread_details
	(
		std::string_view thread_ref,
		std::string_view entity_ref,

		const EntityDescriptor* opt_descriptor=nullptr
	);

	bool parse_implicit_component_settings(const util::json& data);

	std::optional<EntityTarget> resolve_manual_target(const util::json& update_entry);

	std::string resolve_state_name(const util::json& state_data, const std::filesystem::path& state_path);

	// Utility function for loading JSON data.
	std::tuple
	<
		std::filesystem::path, // state_base_path
		std::filesystem::path, // state_path
		util::json             // state_data
	>
	load_state_data
	(
		std::string_view state_path_raw,
		const std::filesystem::path& base_path,
		const EntityFactoryContext* opt_context=nullptr
	);

	// The return value of this function indicates how many components were processed.
	std::size_t process_update_action
	(
		EntityDescriptor& descriptor,
		EntityStateUpdateAction& action_out,
		const util::json& update_entry,
		const MetaParsingContext& opt_parsing_context={}
	);

	std::size_t process_animation_list
	(
		EntityDescriptor& descriptor,
		AnimationRepository& animations_out,
		const util::json& animation_content,

		const MetaParsingContext& opt_parsing_context={}
	);

	std::size_t process_animation_sequence_list
	(
		EntityDescriptor& descriptor,
		AnimationRepository& animations_out,
		const util::json& sequence_list_content,

		const MetaParsingContext& opt_parsing_context={}
	);

	std::size_t process_animation_layer_list
	(
		EntityDescriptor& descriptor,
		AnimationRepository& animations_out,
		const util::json& layer_list_content,

		const MetaParsingContext& opt_parsing_context={}
	);

	std::size_t process_bone_animation_layer_mapping
	(
		EntityDescriptor& descriptor,
		AnimationRepository& animations_out,
		const util::json& bone_mapping_content,

		const MetaParsingContext& opt_parsing_context={}
	);

	std::size_t process_component_list
	(
		EntityDescriptor& descriptor,
		MetaDescription& components_out, // EntityDescriptor::TypeInfo&

		const util::json& components,

		const MetaTypeDescriptorFlags& shared_component_flags={},
		const MetaParsingContext& opt_parsing_context={},

		bool allow_new_entry=true,
		bool allow_default_entries=true,
		bool forward_entry_update_condition_to_flags=false,

		bool ignore_special_symbols=true,

		std::optional<bool> allow_entry_updates=std::nullopt
	);

	// Attempts to process the `component_declaration` and `component_content` specified,
	// adding or modifying an entry in `components_out` if successful.
	// 
	// NOTE: This overload is used internally by `process_component_list`.
	// 
	// Calling this directly can be useful when wanting to process and insert
	// individual component entries that are not necessarily stored in an encompassing JSON container.
	// 
	// e.g. top-level component entries in archetypes, which have non-component
	// names that shouldn't be enumerated by a call to `process_component_list`.
	bool process_component
	(
		EntityDescriptor& descriptor,
		MetaDescription& components_out, // EntityDescriptor::TypeInfo&

		const std::string& component_declaration, // std::string_view

		const util::json* component_content=nullptr,

		const MetaTypeDescriptorFlags& component_flags={},

		const MetaParsingContext& opt_parsing_context={},

		bool allow_new_entry=true,
		bool allow_default_entries=true,
		bool forward_entry_update_condition_to_flags=false,

		bool ignore_special_symbols=true,

		std::optional<bool> allow_entry_update=std::nullopt
	);

	// See simplified overload for details.
	bool process_component
	(
		EntityDescriptor& descriptor,
		MetaDescription& components_out, // EntityDescriptor::TypeInfo&

		const MetaType& component_type,
		bool allow_entry_update=false,
		std::optional<std::uint8_t> constructor_arg_count=std::nullopt, // EntityFactory::SmallSize

		const util::json* component_content=nullptr,

		const MetaTypeDescriptorFlags& component_flags={},

		const MetaParsingContext& opt_parsing_context={},

		bool allow_new_entry=true,
		bool allow_default_entries=true,
		bool forward_entry_update_condition_to_flags=false
	);

	// Same as the `MetaType` overload, but handles resolution of `component_name`.
	std::optional<MetaTypeDescriptor> process_component
	(
		std::string_view component_name,
		const util::json* data,

		std::optional<std::uint8_t> constructor_arg_count=std::nullopt, // SmallSize
		const MetaTypeDescriptorFlags& component_flags={},
		const MetaParsingContext& opt_parsing_context={},
		SharedStorageInterface* opt_storage=nullptr
	);

	// NOTE: The 'allow' arguments refer to construction of the underlying `MetaTypeDescriptor`.
	// To affect the component itself, use the `component_flags` argument.
	MetaTypeDescriptor process_component
	(
		MetaType component_type,
		const util::json* data=nullptr,

		std::optional<std::uint8_t> constructor_arg_count=std::nullopt, // SmallSize
		const MetaTypeDescriptorFlags& component_flags={},
		const MetaParsingContext& opt_parsing_context={},
		SharedStorageInterface* opt_storage=nullptr
	);

	std::size_t process_archetype_component_list
	(
		EntityDescriptor& descriptor,
		const util::json& components,
		const MetaParsingContext& opt_parsing_context={},
		bool change_only_if_missing=false
	);

	// This overload resolves and processes a state from a raw path.
	// 
	// If successful, the value returned by this function
	// is a non-owning pointer to the processed state. (Stored in `states_out`)
	const EntityState* process_state
	(
		EntityDescriptor& descriptor,
		EntityStateCollection& states_out,
		std::string_view state_path_raw, // const std::string&
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,
		std::string_view state_name={}
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
		EntityDescriptor& descriptor,
		EntityStateCollection& states_out,
		const util::json& data,
		std::string_view state_name,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr
	);

	// The `states_out` argument is used only for import resolution.
	// Please use the `state` argument for specifying an existing `EntityState` instance.
	// 
	// The return value of this function indicates if processing was successful.
	//
	// NOTE: This overload does not handle name resolution and assignment. (see `resolve_state_name`)
	bool process_state
	(
		EntityDescriptor& descriptor,
		EntityStateCollection& states_out,
		EntityState& state,
		const util::json& data,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr
	);

	std::size_t process_state_list
	(
		EntityDescriptor& descriptor,
		EntityStateCollection& states_out,
		const util::json& data,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr
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
		EntityDescriptor& descriptor,
		EntityStateCollection& states_out,
		EntityState& state,

		const util::json& data,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr
	);

	// TODO: Optimize to avoid multiple calls to `parse_component_declaration`.
	template <typename Callback>
	inline std::size_t process_and_inspect_component_list
	(
		EntityDescriptor& descriptor,
		
		MetaDescription& components_out,
		const util::json& components,
		
		Callback&& callback,
		
		bool exit_on_false_value=true,

		const MetaTypeDescriptorFlags& shared_component_flags={},
		const MetaParsingContext& opt_parsing_context={},

		bool allow_new_entry=true,
		bool allow_default_entries=true
	)
	{
		process_component_list(descriptor, components_out, components, shared_component_flags, opt_parsing_context, allow_new_entry, allow_default_entries);

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

	std::size_t process_state_isolated_components
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		const util::json& isolated,
		const MetaParsingContext& opt_parsing_context={}
	);

	std::size_t process_state_local_copy_components
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		const util::json& local_copy,
		const MetaParsingContext& opt_parsing_context={}
	);

	std::size_t process_state_init_copy_components
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		const util::json& init_copy,
		const MetaParsingContext& opt_parsing_context={}
	);

	// NOTE: The `opt_states_out` and `opt_base_path` arguments are only used if `allow_inline_import` is enabled.
	std::size_t process_state_rule_list
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		const util::json& rules,

		EntityStateCollection* opt_states_out=nullptr,
		const std::filesystem::path* opt_base_path=nullptr,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,
		bool allow_inline_import=true
	);

	// NOTE: Subroutine of `process_state_rule_list`.
	std::size_t process_state_rule
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		MetaTypeID type_name_id,
		const util::json& content,
				
		std::optional<EventTriggerCondition> condition=std::nullopt,

		EntityStateCollection* opt_states_out=nullptr,
		const std::filesystem::path* opt_base_path=nullptr,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,

		bool allow_inline_import=true
	);

	// NOTE: Subroutine of `process_state_rule_list`.
	std::size_t process_trigger_expression
	(
		EntityDescriptor& descriptor,
		EntityState& state,

		std::string_view trigger_condition_expr,
		const util::json& content,

		EntityStateCollection* opt_states_out=nullptr,
		const std::filesystem::path* opt_base_path=nullptr,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,
		bool allow_inline_import=true
	);

	const EntityState* process_state_inline_import
	(
		EntityDescriptor& descriptor,
		EntityStateCollection* states_out,
		const std::string& command, // std::string_view
		const std::filesystem::path* base_path,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,
		bool allow_inline_import=true
	);

	EntityThreadCount process_state_default_threads
	(
		EntityDescriptor& descriptor,
		EntityState& state,
		
		std::string_view state_name,

		const std::filesystem::path* opt_base_path=nullptr,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr
	);

	EntityThreadCount process_archetype_default_threads
	(
		EntityDescriptor& descriptor,

		const std::filesystem::path& archetype_path,

		const std::filesystem::path* opt_base_path=nullptr,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr
	);

	std::size_t process_archetype_add_implicit_component_from_path
	(
		EntityDescriptor& descriptor,

		const std::filesystem::path& archetype_path,

		const std::filesystem::path* opt_base_path=nullptr,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr
	);

	std::size_t process_state_add_implicit_component_from_name
	(
		EntityDescriptor& descriptor,
		EntityState& state,

		std::string_view state_name,

		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr
	);

	void process_archetype
	(
		EntityDescriptor& descriptor,
		const util::json& data,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,
		bool resolve_external_modules=true,
		std::optional<EntityStateIndex>* opt_default_state_index_out=nullptr
	);

	template <typename ChildFactoryCallback>
	void process_archetype
	(
		EntityDescriptor& descriptor,
		const util::json& data,
		const std::filesystem::path& base_path,
		ChildFactoryCallback&& child_callback,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,
		bool resolve_external_modules=true,
		bool process_children=true,
		std::optional<EntityStateIndex>* opt_default_state_index_out=nullptr
	)
	{
		// Override external module resolution.
		if (resolve_external_modules)
		{
			resolve_archetypes
			(
				descriptor,
				data, base_path,
				child_callback,
				opt_parsing_context, opt_factory_context,
				true, process_children,
				opt_default_state_index_out
			);
		}

		// Execute main overload without external modules.
		process_archetype
		(
			descriptor,
			data, base_path,
			opt_parsing_context, opt_factory_context,
			false,
			opt_default_state_index_out
		);

		if (process_children)
		{
			if (auto children = data.find("children"); children != data.end())
			{
				util::json_for_each<util::json::value_t::string>
				(
					*children,
					[&descriptor, &base_path, opt_factory_context, &child_callback](const auto& value)
					{
						const auto child_path_raw = std::filesystem::path(value.get<std::string>());

						const auto child_path = (opt_factory_context)
							? opt_factory_context->resolve_reference(child_path_raw, base_path)
							: child_path_raw
						;

						child_callback
						(
							descriptor,

							EntityFactoryContext
							{
								{
									.instance_path = child_path,
									.instance_directory = child_path.parent_path(),

									.service_archetype_root_path = (opt_factory_context)
										? opt_factory_context->paths.service_archetype_root_path
										: std::filesystem::path {}
									,

									.archetype_root_path = (opt_factory_context)
										? opt_factory_context->paths.archetype_root_path
										: std::filesystem::path {}
								}
							}
						);
					}
				);
			}
		}
	}

	void process_archetype
	(
		EntityDescriptor& descriptor,
		const std::filesystem::path& archetype_path,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,
		bool resolve_external_modules=true,
		std::optional<EntityStateIndex>* opt_default_state_index_out=nullptr
	);

	template <typename ChildFactoryCallback>
	void process_archetype
	(
		EntityDescriptor& descriptor,
		const std::filesystem::path& archetype_path,
		const std::filesystem::path& base_path,
		ChildFactoryCallback&& child_callback,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,
		bool resolve_external_modules=true,
		bool process_children=true,
		std::optional<EntityStateIndex>* opt_default_state_index_out=nullptr
	)
	{
		if (archetype_path.empty())
		{
			return;
		}

		// TODO: Optimize via caching, etc.
		auto instance = util::load_json(archetype_path);

		process_archetype
		(
			descriptor, instance, base_path, child_callback,
			opt_parsing_context, opt_factory_context,
			resolve_external_modules, process_children,
			opt_default_state_index_out
		);

		process_archetype_default_threads
		(
			descriptor, archetype_path,
			&base_path, opt_parsing_context, opt_factory_context
		);

		process_archetype_add_implicit_component_from_path
		(
			descriptor, archetype_path,
			&base_path, opt_parsing_context, opt_factory_context
		);
	}

	bool resolve_archetypes
	(
		EntityDescriptor& descriptor,
		const util::json& instance,
		const std::filesystem::path& base_path,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,
		bool resolve_external_modules=true,
		std::optional<EntityStateIndex>* opt_default_state_index_out=nullptr
	);

	// Resolves the contents of the `archetypes` field pointed to by `instance`.
	// 
	// If an `archetypes` field is not present in `instance`,
	// or if its contents could not be read, this will return false.
	// 
	// See also: `process_archetype`.
	template <typename ChildFactoryCallback>
	bool resolve_archetypes
	(
		EntityDescriptor& descriptor,
		const util::json& instance,
		const std::filesystem::path& base_path,
		ChildFactoryCallback&& child_callback,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,
		bool resolve_external_modules=true, bool process_children=true,
		std::optional<EntityStateIndex>* opt_default_state_index_out=nullptr
	)
	{
		auto archetypes = util::find_any(instance, "archetypes", "import", "imports", "modules", "merge");

		if (archetypes == instance.end())
		{
			return false;
		}

		auto elements_processed = std::size_t {};

		const auto implicit_components = parse_implicit_component_settings(instance);

		if (implicit_components)
		{
			const auto components_processed = process_archetype_component_list(descriptor, *archetypes, opt_parsing_context, true);

			elements_processed += components_processed;
		}

		// TODO: Add support for JSON objects as input. (i.e. embedded archetypes)
		const auto archetypes_processed = util::json_for_each<util::json::value_t::string>
		(
			*archetypes,

			[&descriptor, &base_path, &child_callback, opt_parsing_context, opt_factory_context, resolve_external_modules, process_children, &opt_default_state_index_out](const auto& value)
			{
				const auto& archetype_path_raw_str = value.get<std::string>();
				const auto archetype_path_raw = std::filesystem::path(archetype_path_raw_str);
				
				const auto archetype_path = (opt_factory_context)
					? opt_factory_context->resolve_reference(archetype_path_raw, base_path)
					: archetype_path_raw
				;

				if (archetype_path.empty())
				{
					// If an archetype couldn't be found, try to process
					// the reference as a path to a thread instead:
					process_archetype_default_threads
					(
						descriptor, archetype_path_raw,
						&base_path, opt_parsing_context, opt_factory_context
					);
				}
				else
				{	
					const auto base_path = archetype_path.parent_path();

					process_archetype
					(
						descriptor,
						archetype_path,
						base_path,
						child_callback,
						opt_parsing_context, opt_factory_context,
						resolve_external_modules, process_children,
						opt_default_state_index_out
					);
				}
			}
		);

		elements_processed += archetypes_processed;

		return (elements_processed > 0);
	}
}