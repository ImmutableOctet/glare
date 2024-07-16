#include "entity_thread_builder.hpp"
#include "entity_thread_description.hpp"
#include "entity_factory_context.hpp"
#include "entity_descriptor.hpp"
#include "entity_descriptor_shared.hpp"

#include "serial.hpp"
#include "serial_impl.hpp"

#include <engine/timer.hpp>

//#include <engine/meta/meta.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/hash.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/data_member.hpp>
#include <engine/meta/enum.hpp>

#include <engine/meta/meta_type_resolution_context.hpp>
#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/meta_value_operation.hpp>
#include <engine/meta/meta_value_operator.hpp>
#include <engine/meta/indirect_meta_any.hpp>
#include <engine/meta/indirect_meta_variable_target.hpp>
#include <engine/meta/meta_variable_context.hpp>
#include <engine/meta/meta_variable_target.hpp>
#include <engine/meta/meta_property.hpp>

#include <engine/reflection/script.hpp>

#include <util/string.hpp>
#include <util/parse.hpp>
#include <util/io.hpp>
#include <util/optional.hpp>
#include <util/variant.hpp>

#include <tuple>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	struct OnThreadSpawn;

	template <typename InstructionType>
	static std::optional<IndirectMetaAny> resolve_instruction_impl // std::optional<EntityDescriptorShared<MetaTypeDescriptor>>
	(
		EntityThreadBuilder& self,
		const std::optional<EntityThreadInstruction>& opt_thread_instruction,
		std::string_view instruction_content
	)
	{
		using namespace engine::literals;

		auto type = resolve<InstructionType>();

		if (!type)
		{
			return std::nullopt;
		}

		auto type_desc = MetaTypeDescriptor { type };

		std::size_t argument_offset = 0;

		if constexpr (std::is_base_of_v<EntityThreadInstruction, InstructionType>)
		{
			//if (opt_thread_instruction)
			{
				type_desc.set_variable
				(
					MetaVariable
					{
						"target_entity"_hs,

						(opt_thread_instruction)
							? opt_thread_instruction->target_entity
							: EntityTarget {}
								
						//opt_thread_instruction->target_entity
					}
				);

				type_desc.set_variable
				(
					MetaVariable
					{
						"thread_id"_hs,

						(opt_thread_instruction)
							? opt_thread_instruction->thread_id
							: EntityThreadID {}

						//opt_thread_instruction->thread_id
					}
				);
			}

			argument_offset = 2;
		}

		auto& descriptor = self.get_descriptor();

		type_desc.set_variables
		(
			instruction_content,

			MetaParsingInstructions
			{
				.context = self.get_parsing_context(),
				.storage = &(descriptor.get_shared_storage()),

				.fallback_to_string               = false,
				.fallback_to_component_reference  = true,
				.fallback_to_entity_reference     = true,

				.allow_member_references          = true,
				.allow_entity_indirection         = true,
				.allow_implicit_type_construction = true, // false,
				.allow_explicit_type_construction = true,
				.allow_numeric_literals           = false,
				.allow_function_call_semantics    = true,
				.allow_value_resolution_commands  = true,
				.allow_remote_variable_references = true,

				.resolve_component_aliases        = true,
				.resolve_command_aliases          = false, // true,
				.resolve_instruction_aliases      = true
			},

			argument_offset
		);

		if (type_desc.has_indirection(true))
		{
			auto resource = descriptor.allocate(std::move(type_desc));

			//const auto descriptor_type = resolve<MetaTypeDescriptor>();
			const auto descriptor_type_id = hash("MetaTypeDescriptor").value(); // descriptor_type.id();

			const auto checksum = descriptor.get_shared_storage().get_checksum(descriptor_type_id);

			return IndirectMetaAny
			(
				static_cast<const util::SharedStorageRef<MetaTypeDescriptor, SharedStorageIndex>&>(resource),
				descriptor_type_id,
				checksum
			);
		}

		return std::nullopt;
	}

	// EntityThreadBuilderContext:
	EntityThreadBuilderContext::EntityThreadBuilderContext
	(
		EntityDescriptor& descriptor,
		ThreadDescriptor thread,
		const EntityFactoryContext* opt_factory_context,
		const std::filesystem::path* opt_base_path,
		const MetaParsingContext& parsing_context,
		EntityThreadCadence cadence
	) :
		descriptor(descriptor),
		opt_factory_context(opt_factory_context),
		opt_base_path(opt_base_path),
		parsing_context(parsing_context),
		cadence(cadence),
		thread(thread)
	{}

	EntityThreadBuilderContext::EntityThreadBuilderContext
	(
		const EntityThreadBuilderContext& parent_context,
		const MetaParsingContext& parsing_context
	) :
		EntityThreadBuilderContext(parent_context)
	{
		this->parsing_context = parsing_context;
	}

	EntityThreadBuilderContext::EntityThreadBuilderContext
	(
		const EntityThreadBuilderContext& parent_context,
		EntityThreadCadence cadence
	) :
		EntityThreadBuilderContext(parent_context)
	{
		this->cadence = cadence;
	}

	EntityThreadBuilderContext::EntityThreadBuilderContext
	(
		const EntityThreadBuilderContext& parent_context,
		const MetaParsingContext& parsing_context,
		EntityThreadCadence cadence
	) :
		EntityThreadBuilderContext(parent_context, parsing_context)
	{
		this->cadence = cadence;
	}

	EntityThreadIndex EntityThreadBuilderContext::thread_index() const
	{
		//return descriptor.shared_storage.get_index_safe(thread);

		return thread.get_index();
	}

	// EntityThreadBuilder:
	std::string EntityThreadBuilder::thread_name_from_script_reference
	(
		const std::filesystem::path& script_path,

		const EntityFactoryContext* opt_factory_context,
		const std::filesystem::path* opt_base_path
	)
	{
		/*
		// Alternative implementation
		// (Disabled for now; path resolution does not change the trailing filename at this time):
		if (opt_factory_context)
		{
			auto resolved_path = opt_factory_context->resolve_script_reference
			(
				script_path,

				(
					(opt_base_path)
					? *opt_base_path
					: std::filesystem::path {}
				)
			);

			return thread_name_from_resolved_path(resolved_path);
		}

		return thread_name_from_resolved_path(script_path);
		*/

		// Most optimal control-path currently is to skip path resolution. (See notes above)
		return thread_name_from_resolved_path(script_path);
	}

	std::string EntityThreadBuilder::thread_name_from_script_reference
	(
		const std::string& script_path_str,
		const EntityFactoryContext* opt_factory_context,
		const std::filesystem::path* opt_base_path
	)
	{
		return thread_name_from_script_reference(std::filesystem::path { script_path_str }, opt_factory_context, opt_base_path);
	}

	std::string EntityThreadBuilder::thread_name_from_script_reference
	(
		std::string_view script_path_str,
		const EntityFactoryContext* opt_factory_context,
		const std::filesystem::path* opt_base_path
	)
	{
		return thread_name_from_script_reference(std::string { script_path_str }, opt_factory_context, opt_base_path);
	}

	std::string EntityThreadBuilder::thread_name_from_resolved_path(const std::filesystem::path& resolved_path)
	{
		auto filename = resolved_path.filename();
		
		filename.replace_extension();

		return filename.string();
	}

	bool EntityThreadBuilder::is_yield_instruction(std::string_view instruction_name)
	{
		return is_yield_instruction(hash(instruction_name).value());
	}

	bool EntityThreadBuilder::is_yield_instruction(MetaSymbolID instruction_id)
	{
		using namespace engine::literals;

		switch (instruction_id)
		{
			case "sleep"_hs:
			case "wait"_hs:
			case "yield"_hs:
				return true;
		}

		return false;
	}

	bool EntityThreadBuilder::is_event_capture_instruction(std::string_view instruction_name)
	{
		return is_event_capture_instruction(hash(instruction_name).value());
	}

	bool EntityThreadBuilder::is_event_capture_instruction(MetaSymbolID instruction_id)
	{
		using namespace engine::literals;

		switch (instruction_id)
		{
			case "capture"_hs:
			case "event"_hs:
			case "event_capture"_hs:
				return true;
		}

		return false;
	}

	EntityThreadBuilder::EntityThreadBuilder
	(
		const EntityThreadBuilderContext& context,
		std::string_view opt_thread_name
	)
		: EntityThreadBuilderContext(context)
	{
		set_thread_name(opt_thread_name);
	}

	EntityThreadBuilder::EntityThreadBuilder
	(
		EntityDescriptor& descriptor,
		ThreadDescriptor thread,
		std::string_view opt_thread_name,
		const EntityFactoryContext* opt_factory_context,
		const std::filesystem::path* opt_base_path,
		const MetaParsingContext& parsing_context,
		EntityThreadCadence cadence
	)
		: EntityThreadBuilder
		(
			EntityThreadBuilderContext
			(
				descriptor,
				thread,
				opt_factory_context,
				opt_base_path,
				parsing_context,
				cadence
			),

			opt_thread_name
		)
	{}

	EntityThreadBuilder::EntityThreadBuilder
	(
		EntityDescriptor& descriptor,
		std::string_view opt_thread_name,
		const EntityFactoryContext* opt_factory_context,
		const std::filesystem::path* opt_base_path,
		const MetaParsingContext& parsing_context,
		EntityThreadCadence cadence
	)
		: EntityThreadBuilder
		(
			descriptor,
			
			ThreadDescriptor
			(
				descriptor,
				descriptor.shared_storage.allocate<EntityThreadDescription>(cadence)
			),

			opt_thread_name,
			opt_factory_context,
			opt_base_path,
			parsing_context,
			cadence
		)
	{}

	EntityThreadBuilder::~EntityThreadBuilder() {}

	EntityThreadID EntityThreadBuilder::set_thread_name(std::string_view thread_name, bool resolve_name, bool force)
	{
		auto& thread = get_thread();

		if ((!force) && (thread.thread_id))
		{
			return {}; // thread.thread_id;
		}

		if (thread_name.empty())
		{
			return {};
		}

		EntityThreadID thread_id_out = {};

		if (resolve_name)
		{
			const auto result = peek_string_value
			(
				thread_name,

				[&thread_id_out, force](const auto& resolved_name)
				{
					if (resolved_name.empty())
					{
						return;
					}
					
					thread_id_out = hash(resolved_name).value();
				},

				{
					// Ensure we're resolving this as a proxy-to-string.
					.resolve_symbol = true,

					// Already handled in previous step.
					.strip_quotes   = false,

					// Allows fallthrough to non-resolution path below.
					.fallback_to_string               = false,
					.fallback_to_component_reference  = false,
					.fallback_to_entity_reference     = false,

					// We determine thread names during the initial processing phase,
					// and will therefore have no use for runtime name resolution:
					.allow_member_references          = false,
					.allow_entity_indirection         = false,
					.allow_remote_variable_references = false // true
				}
			);
		}

		if (!thread_id_out)
		{
			thread_id_out = hash(thread_name).value();
		}

		assert(thread_id_out);

		// Check for an existing thread with the proposed ID.
		const auto thread_id_already_exists = static_cast<bool>(descriptor.get_thread_index(thread_id_out));

		//assert(!thread_id_already_exists);

		if (thread_id_already_exists)
		{
			print_warn("Failed to set thread ID to #{}: An existing thread already has that name. (Input: \"{}\")", thread_id_out, thread_name);

			return {};
		}
		//else
		{
			if (thread_id_out)
			{
				thread.thread_id = thread_id_out;

				if (auto opt_variable_context = parsing_context.get_variable_context())
				{
					opt_variable_context->set_name(thread_id_out);
				}
			}
		}

		//return thread_id_out;
		return thread.thread_id;
	}

	EntityThreadID EntityThreadBuilder::get_thread_name() const
	{
		return get_thread().thread_id;
	}

	bool EntityThreadBuilder::thread_has_name() const
	{
		return static_cast<bool>(get_thread_name());
	}

	EntityThreadBuilder::InstructionIndex EntityThreadBuilder::get_instruction_index()
	{
		return static_cast<InstructionIndex>(get_thread().instructions.size());
	};

	EntityStateUpdateAction& EntityThreadBuilder::get_update_instruction(EntityTarget target)
	{
		// Check if we're already building an update instruction:
		if (current_update_instruction)
		{
			// Determine if we're still targeting the same entity:
			if (current_update_instruction->target_entity == target)
			{
				// Keep using the current update instruction.
				return *current_update_instruction;
			}
			
			// Flush the existing update instruction,
			// since we're updating a different entity.
			flush_update_instruction();
		}

		return construct_update_instruction(target);
	}

	EntityStateUpdateAction& EntityThreadBuilder::construct_update_instruction(EntityTarget target)
	{
		current_update_instruction = EntityStateUpdateAction { target };

		return *current_update_instruction;
	}

	bool EntityThreadBuilder::flush_update_instruction()
	{
		if (!current_update_instruction)
		{
			return false;
		}

		emit(std::move(*current_update_instruction));

		current_update_instruction = std::nullopt;

		return true;
	}

	EntityStateUpdateAction& EntityThreadBuilder::new_update_instruction(EntityTarget target)
	{
		if (current_update_instruction)
		{
			auto flush_result = flush_update_instruction();

			assert(flush_result);
		}

		return construct_update_instruction(target);
	}

	bool EntityThreadBuilder::on_update_instruction(const util::json& update_entry)
	{
		if (current_update_instruction)
		{
			// TODO: Look into methods of avoiding extra check and parse operation.
			// 
			// Check if `update_entry` has a manual target specified:
			if (auto manual_target = resolve_manual_target(update_entry))
			{
				// If this update instruction is targeting a different entity than the currently
				// established update instruction, flush that operation and begin anew.
				if (manual_target != current_update_instruction->target_entity)
				{
					// Different target entity; can't combine into one operation.
					flush_update_instruction();
				}
			}
			else
			{
				// Since there's no manual target specified, the implied target is `SelfTarget`.
				// Check against the current update operation to see if it's also affecting `SelfTarget`.
				if (!current_update_instruction->target_entity.is_self_targeted())
				{
					// This new instruction doesn't seem to be related to the previous (i.e. active)
					// update instruction, we'll flush that operation to be safe.
					flush_update_instruction();
				}
			}
		}

		auto& update_instruction = get_update_instruction();

		return (process_update_action(descriptor, update_instruction, update_entry, parsing_context) > 0);
	}

	bool EntityThreadBuilder::on_instruction_change(InstructionID instruction_id, InstructionID prev_instruction_id)
	{
		using namespace engine::literals;

		this->prev_instruction_id = instruction_id;

		switch (prev_instruction_id)
		{
			case "update"_hs:
				return static_cast<bool>(flush_update_instruction());
		}

		return false;
	};

	bool EntityThreadBuilder::process_update_instruction_from_values
	(
		std::string_view type_name,
		std::string_view member_name,
		std::string_view assignment_value_raw,
		std::string_view entity_ref_expr,
		std::string_view operator_symbol
	)
	{
		const auto target = (entity_ref_expr.empty())
			? EntityTarget { EntityTarget::SelfTarget {} }
			: EntityTarget::from_string(entity_ref_expr)
		;

		return process_update_instruction_from_values
		(
			type_name,
			member_name,
			assignment_value_raw,
			operator_symbol,
			target
		);
	}

	bool EntityThreadBuilder::process_update_instruction_from_values
	(
		std::string_view type_name,
		std::string_view member_name,
		std::string_view assignment_value_raw,
		std::string_view operator_symbol,

		const EntityTarget& target
	)
	{
		using namespace engine::literals;

		if (target.is_null_target())
		{
			return {};
		}

		if (type_name.empty())
		{
			//print_error("Missing type-name detected.");

			return {};
		}

		const auto opt_type_context = get_type_context();

		const auto type = (opt_type_context)
			? opt_type_context->get_component_type(type_name)
			: resolve(hash(type_name))
		;

		if (!type)
		{
			//print_error("Unable to resolve type: {} (#{})", type_name, type.id());

			return {};
		}

		const auto type_id = type.id();

		const auto assignment_symbol_index = operator_symbol.find_last_of('=');

		if (assignment_symbol_index == std::string_view::npos)
		{
			return {};
		}

		// Retrieve the current update instruction, or
		// initialize a new one if one doesn't exist yet.
		auto& update_instruction = get_update_instruction(target);

		auto* target_descriptor = update_instruction.updated_components.get_definition(descriptor, type_id);

		const auto member_id = (member_name.empty())
			? MetaTypeID {}
			: hash(member_name).value()
		;

		if (target_descriptor)
		{
			bool force_flush = !(static_cast<bool>(member_id));

			if (!force_flush)
			{
				if (target_descriptor->has_anonymous_field_name() || target_descriptor->get_variable(member_id))
				{
					force_flush = true;
				}
			}

			if (force_flush)
			{
				// Since we've already assigned `member_name`, we'll need to
				// flush the current update instruction and start a new one:

				// NOTE: There's no need to re-assign `update_instruction` here,
				// since the new instance should be in the same memory location.
				new_update_instruction(target);

				// Ensure a new descriptor is allocated. (see below)
				target_descriptor = nullptr;
			}
		}
		
		if (!target_descriptor)
		{
			target_descriptor = &
			(
				update_instruction.updated_components.type_definitions.emplace_back
				(
					descriptor.allocate<MetaTypeDescriptor>
					(
						type,
						std::optional<MetaTypeDescriptor::SmallSize>(std::nullopt),

						MetaTypeDescriptorFlags
						{
							// Force field assignment for member-based inline update instructions.
							.force_field_assignment = static_cast<bool>(member_id)
						}
					)
				).get(descriptor)
			);
		}

		const auto operator_type_raw = operator_symbol.substr(0, assignment_symbol_index);
		const auto operator_info = MetaValueOperation::get_operation(operator_type_raw);

		auto member = (member_id)
			? resolve_data_member_by_id(type, true, member_id)
			: entt::meta_data {}
		;

		//assert(member);

		if ((member_id) && (!member))
		{
			return false;
		}

		auto assignment_value = meta_any_from_string
		(
			assignment_value_raw,

			{
				.context = parsing_context,
				.storage = &(descriptor.get_shared_storage()),

				// TODO: Add support for direct component assignment/copying.
				.fallback_to_component_reference  = true,
				.fallback_to_entity_reference     = true,

				.allow_member_references          = true,
				.allow_entity_indirection         = true,
				.allow_function_call_semantics    = true,
				.allow_value_resolution_commands  = true,
				.allow_remote_variable_references = true,
				.resolve_value_operations         = true,
			},

			(member)
				? member.type()
				: type // MetaType {} 
		);

		if (operator_info)
		{
			const auto& operator_type = std::get<0>(*operator_info);

			auto& operation = descriptor.get_shared_storage().allocate<MetaValueOperation>();

			operation.segments.emplace_back
			(
				IndirectMetaDataMember
				{
					target,

					MetaDataMember
					{
						type_id,

						member_id
					}
				},

				operator_type
			);

			operation.segments.emplace_back
			(
				std::move(assignment_value),

				operator_type
			);

			target_descriptor->set_variable
			(
				MetaVariable
				{
					member_id,

					IndirectMetaAny
					{
						descriptor.get_shared_storage(),
						operation
					}
				}
			);
		}
		else
		{
			target_descriptor->set_variable(MetaVariable { member_id, std::move(assignment_value) });
		}

		return static_cast<bool>(target_descriptor);
	}

	bool EntityThreadBuilder::process_update_instruction_from_csv(std::string_view instruction_separated, std::string_view separator)
	{
		if (auto values = util::split_from<4>(instruction_separated, separator, 3))
		{
			return std::apply
			(
				[this](auto&&... args)
				{
					return process_update_instruction_from_values(std::forward<decltype(args)>(args)...);
				},

				*values
			);
		}

		return {};
	}

	bool EntityThreadBuilder::process_inline_update_instruction(std::string_view instruction)
	{
		auto
		[
			entity_ref_expr,
			type_name, member_name,
			operator_symbol, assignment_value_raw,
			updated_offset
		] = parse_qualified_assignment_or_comparison(instruction);

		return process_update_instruction_from_values(type_name, member_name, assignment_value_raw, entity_ref_expr, operator_symbol);
	};

	EntityInstructionCount EntityThreadBuilder::process_meta_expression_instruction
	(
		std::string_view instruction,
		
		bool allow_first_symbol_entity_fallback,
		bool allow_leading_remote_variable
	)
	{
		using namespace engine::literals;
		using namespace engine::instructions;

		const MetaVariableDescription* assignment_variable = {};

		if (auto opt_variable_context = parsing_context.get_variable_context())
		{
			auto [assignment_operator_index, assignment_operator] = util::find_assignment_operator(instruction);

			if (!assignment_operator.empty())
			{
				auto unresolved_variable_name = instruction.substr(0, assignment_operator_index);

				if (auto type_operator = unresolved_variable_name.find_last_of(':'); type_operator != std::string_view::npos)
				{
					bool type_operator_is_valid = true;

					if (auto scope_end = unresolved_variable_name.find_last_of(')'); scope_end != std::string_view::npos)
					{
						if (scope_end > type_operator)
						{
							type_operator_is_valid = false;
						}
					}

					if (type_operator_is_valid)
					{
						unresolved_variable_name = unresolved_variable_name.substr(0, type_operator);
					}
				}

				unresolved_variable_name = util::trim(unresolved_variable_name);

				assignment_variable = opt_variable_context->retrieve_variable(unresolved_variable_name);

				if (assignment_variable)
				{
					const auto assignment_value_raw = instruction.substr(assignment_operator_index + assignment_operator.length());
					const auto assignment_value_as_command = util::parse_command(assignment_value_raw, false, false, false, false);

					if (const auto& assignment_command_name = std::get<0>(assignment_value_as_command); !assignment_command_name.empty())
					{
						if (const auto& assignment_command_content = std::get<1>(assignment_value_as_command); !assignment_command_content.empty())
						{
							const auto instruction_id = hash(assignment_command_name).value();

							auto event_type_id = assignment_variable->type_id;

							bool variable_assignment_is_capture = false;

							if (is_yield_instruction(instruction_id))
							{
								const auto inline_yield_result = process_inline_yield_instruction(assignment_command_content);

								assert(inline_yield_result);

								if (inline_yield_result)
								{
									variable_assignment_is_capture = true;
								}
							}
							else if (is_event_capture_instruction(instruction_id))
							{
								const auto latest_instruction = get_latest_instruction();

								constexpr auto yield_index = util::variant_index<EntityInstruction::InstructionType, engine::instructions::Yield>();
								constexpr auto event_capture_index = util::variant_index<EntityInstruction::InstructionType, engine::instructions::EventCapture>();

								switch (latest_instruction->type_index())
								{
									case yield_index:
									case event_capture_index:
										if (assignment_command_content.empty())
										{
											variable_assignment_is_capture = true;
										}
										else
										{
											const auto opt_type_context = parsing_context.get_type_context();

											const auto event_type = (opt_type_context)
												? opt_type_context->get_type(assignment_command_content)
												: resolve(hash(assignment_command_content).value())
											;

											if (event_type)
											{
												event_type_id = event_type.id();

												variable_assignment_is_capture = true;
											}
											else
											{
												print_warn("Event capture failed: Unable to resolve event type.");
											}
										}

										break;

									default:
										print_warn("Event capture failed: Event capture instructions are only allowed immediately after a yield instruction. (Continuing as normal assignment)");

										break;
								}
							}

							if (variable_assignment_is_capture)
							{
								instruct<EventCapture>
								(
									MetaVariableTarget
									{
										assignment_variable->resolved_name,
										assignment_variable->scope
									},

									event_type_id
								);

								// Only one instruction processed, but several produced.
								return 1;
							}
						}
					}
				}
			}
		}

		auto& storage = descriptor.get_shared_storage();

		auto deferred_operation = meta_any_from_string
		(
			instruction,

			{
				.context = parsing_context,
				.storage = &storage,

				.fallback_to_component_reference  = true,
				.fallback_to_entity_reference     = true,

				.allow_member_references          = true,
				.allow_entity_indirection         = true,
				.allow_function_call_semantics    = true,
				.allow_value_resolution_commands  = true,
				.allow_remote_variable_references = true,
				.resolve_value_operations         = true
			},

			{},

			// Disabled so that we fail on unresolved symbols.
			false, // allow_string_fallback
			
			// TODO: Determine if these should be disabled for the initial value as well.
			true, // allow_numeric_literals
			true, // allow_boolean_literals
			
			// Defaults to being disabled on the initial value to allow for fallthrough to thread operations.
			// (Similar to reasoning for `allow_string_fallback` always being false)
			allow_first_symbol_entity_fallback, // allow_entity_fallback

			false, // allow_component_fallback
			false, // allow_standalone_opaque_function

			// NOTE: Generally speaking, this value should be the inverse of `allow_first_symbol_entity_fallback`
			// to ensure entity names are resolved correctly. (see default argument)
			// (This is because remote variables are detected earlier in the process and would take priority otherwise)
			allow_leading_remote_variable // (!allow_first_symbol_entity_fallback) // allow_remote_variables
		);

		if (!deferred_operation)
		{
			return 0;
		}

		auto* as_indirect = deferred_operation.try_cast<IndirectMetaAny>();

		if (!as_indirect)
		{
			return 0;
		}

		auto remote_instance = as_indirect->get(storage);

		if (!remote_instance)
		{
			return 0;
		}

		if (assignment_variable)
		{
			return instruct_thread<VariableAssignment>
			(
				std::nullopt,
				std::move(*as_indirect),

				MetaVariableTarget
				{
					assignment_variable->resolved_name,
					assignment_variable->scope
				}
			);
		}
		else
		{
			if (const auto* as_operation = remote_instance.try_cast<MetaValueOperation>())
			{
				// TODO: Look into expanding this check to only include function calls without non-`MetaValueOperator::Get` operators.
				if (as_operation->contains_function_call(storage, true))
				{
					return instruct<FunctionCall>(std::move(*as_indirect));
				}
			}

			return instruct<AdvancedMetaExpression>(std::move(*as_indirect));
		}
	}

	EntityInstructionCount EntityThreadBuilder::process_remote_variable_assignment
	(
		const EntityThreadInstruction& thread_instruction,

		std::string_view thread_name,
		std::string_view thread_local_variable_name,
		std::string_view assignment_value_raw,
		std::string_view operator_symbol,
		std::string_view opt_entity_ref_expr
	)
	{
		using namespace engine::instructions;

		// NOTE: May change this logic later. (see also: `process_trigger_expression`)
		auto thread_local_variable_scope = (thread_instruction.thread_id || thread_instruction.target_entity.is_self_targeted())
			? MetaVariableScope::Local
			: MetaVariableScope::Global
		;

		const auto thread_local_variable_target = MetaVariableTarget
		{
			MetaVariableContext::resolve_path
			(
				static_cast<MetaSymbolID>(thread_instruction.thread_id),

				thread_local_variable_name,
				thread_local_variable_scope
			),

			thread_local_variable_scope
		};

		auto& storage = descriptor.get_shared_storage();

		auto assignment_value = meta_any_from_string
		(
			assignment_value_raw,

			{
				.context = parsing_context,
				.storage = &storage,

				.fallback_to_component_reference  = true,
				.fallback_to_entity_reference     = true,
				.allow_member_references          = true,
				.allow_entity_indirection         = true,
				.allow_function_call_semantics    = true,
				.allow_value_resolution_commands  = true,
				.allow_remote_variable_references = true,
				.resolve_value_operations         = true
			},

			{}, false
		);

		if (!assignment_value)
		{
			return 0;
		}

		const auto operator_parse_result = parse_value_operator(operator_symbol);

		if (!operator_parse_result)
		{
			return 0;
		}

		const auto& operator_value = std::get<0>(*operator_parse_result);

		if (!is_assignment_operation(operator_value))
		{
			return 0;
		}

		auto operation_out = MetaValueOperation {};

		operation_out.segments.emplace_back
		(
			allocate_meta_any
			(
				IndirectMetaVariableTarget
				{
					thread_instruction.target_entity,
					
					thread_local_variable_target,

					(thread_instruction.thread_id)
						? EntityThreadTarget { thread_instruction.thread_id }
						: EntityThreadTarget {}
				},

				&storage
			),

			operator_value
		);

		operation_out.segments.emplace_back(std::move(assignment_value), operator_value);

		auto allocation_result = allocate_meta_any(std::move(operation_out), &storage);

		if (!allocation_result)
		{
			return 0;
		}

		auto* as_indirect = allocation_result.try_cast<IndirectMetaAny>();

		if (!as_indirect)
		{
			return 0;
		}

		return instruct_thread<VariableAssignment>
		(
			thread_instruction,
			std::move(*as_indirect),
			thread_local_variable_target
		);
	}

	EntityInstructionCount EntityThreadBuilder::process_command_instruction
	(
		InstructionID instruction_id,
		std::string_view instruction_name,
		std::string_view instruction_content,

		bool resolve_values
	)
	{
		auto command_type = resolve(instruction_id);

		if (!command_type)
		{
			if (const auto opt_type_context = get_type_context())
			{
				command_type = opt_type_context->get_command_type_from_alias(instruction_name);

				if (!command_type)
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}

		auto& command_content = descriptor.generate_empty_command(command_type); // auto

		command_content.set_variables
		(
			instruction_content,
				
			{
				.context = parsing_context,
				.storage = &(descriptor.get_shared_storage()),

				.resolve_symbol = resolve_values,

				.fallback_to_component_reference = true,
				.fallback_to_entity_reference    = true,

				.allow_member_references          = resolve_values,
				.allow_entity_indirection         = resolve_values,
				.allow_remote_variable_references = resolve_values
			},
			
			command_arg_offset
		);

		return instruct
		(
			EntityStateCommandAction
			{
				{ descriptor, command_content } // std::move(command_content)
			}
		);
	}

	std::optional<EntityInstructionCount> EntityThreadBuilder::process_directive
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,

		const std::optional<EntityThreadInstruction>& thread_details,

		std::string_view directive
	)
	{
		directive = util::trim(directive);

		auto directive_id = StringHash {};
		auto directive_content = std::string_view {};

		if (auto whitespace = directive.find_first_of(util::whitespace_symbols); whitespace != std::string_view::npos)
		{
			const auto directive_name = directive.substr(0, whitespace);

			directive_id = hash(directive_name).value();
			directive_content = util::trim(directive.substr(whitespace));
		}
		else
		{
			directive_id = hash(directive).value();
		}

		return process_directive_impl
		(
			content_source,
			content_index,

			thread_details,

			directive,
			directive_id,
			directive_content
		);
	}

	std::optional<EventTriggerCondition> EntityThreadBuilder::process_immediate_trigger_condition(std::string_view trigger_condition_expr)
	{
		if (auto condition = process_unified_condition_block(descriptor, trigger_condition_expr, parsing_context))
		{
			return condition;
		}

		auto& shared_storage = descriptor.get_shared_storage();

		auto as_embedded_expr = meta_any_from_string
		(
			trigger_condition_expr,

			{
				.context = parsing_context,
				.storage = &shared_storage,

				//.fallback_to_component_reference = false,

				// TODO: Determine if these should be disabled:
				.fallback_to_component_reference  = true, // false,
				.fallback_to_entity_reference     = true, // false,

				.allow_member_references          = true,
				.allow_entity_indirection         = true,
				.allow_remote_variable_references = true
			}
			//, resolve<bool>()
		);

		if (as_embedded_expr)
		{
			return EventTriggerCondition
			{
				EventTriggerSingleCondition
				{
					std::move(as_embedded_expr)
				}
			};
		}

		return std::nullopt;
	}

	EntityInstructionCount EntityThreadBuilder::process_inline_yield_instruction(std::string_view yield_instruction_content)
	{
		using namespace engine::instructions;

		if (auto condition = process_unified_condition_block(descriptor, yield_instruction_content, parsing_context))
		{
			auto condition_ref = descriptor.allocate<EventTriggerCondition>(std::move(*condition));

			return instruct_thread<Yield>(std::nullopt, std::move(condition_ref));
		}
		else if (auto remote_instruction = resolve_instruction_impl<Sleep>(*this, std::nullopt, yield_instruction_content))
		{
			return instruct<InstructionDescriptor>(std::move(*remote_instruction));
		}

		return 0;
	}

	EntityInstructionCount EntityThreadBuilder::process_variable_declaration(std::string_view declaration)
	{
		using namespace engine::instructions;

		auto variable_context = get_variable_context();

		if (!variable_context)
		{
			return 0;
		}

		auto
		[
			scope_qualifier,
			variable_name,
			variable_type_specification,
			assignment_expr,
			trailing_expr
		] = util::parse_variable_declaration(declaration);

		if (variable_name.empty())
		{
			return 0;
		}

		const auto variable_scope = parse_variable_scope(scope_qualifier);

		if (!variable_scope)
		{
			return 0;
		}

		const auto opt_type_context = get_type_context();

		MetaType variable_type;

		if (!variable_type_specification.empty())
		{
			variable_type = (opt_type_context)
				? opt_type_context->get_type(variable_type_specification)
				: resolve(hash(variable_type_specification))
			;

			if (!variable_type)
			{
				return 0;
			}
		}

		const bool is_shared_variable = (*variable_scope != MetaVariableScope::Local);

		auto [variable_description, variable_defined] = variable_context->define_or_retrieve_variable
		(
			*variable_scope,
			variable_name,
			((variable_type) ? variable_type.id() : MetaTypeID {}),
			is_shared_variable
		);

		if (!variable_description)
		{
			return 0;
		}

		if (is_shared_variable || variable_defined)
		{
			if (assignment_expr.empty())
			{
				return instruct<VariableDeclaration>
				(
					MetaVariableTarget
					{
						variable_description->resolved_name,
						variable_description->scope
					}
				);
			}
			else
			{
				const auto assignment_expr_as_command = util::parse_command(assignment_expr, false, false, false, false);

				if (const auto& assignment_expr_command_name = std::get<0>(assignment_expr_as_command); !assignment_expr_command_name.empty())
				{
					if (const auto& assignment_expr_content = std::get<1>(assignment_expr_as_command); !assignment_expr_content.empty())
					{
						const auto instruction_id = hash(assignment_expr_command_name).value();

						auto event_type_id = MetaTypeID {};

						bool variable_assignment_is_capture = false;

						if (is_yield_instruction(instruction_id))
						{
							instruct<VariableDeclaration>
							(
								MetaVariableTarget
								{
									variable_description->resolved_name,
									variable_description->scope
								}
							);

							const auto inline_yield_result = process_inline_yield_instruction(assignment_expr_content);

							assert(inline_yield_result);

							if (inline_yield_result)
							{
								event_type_id = variable_description->type_id;

								variable_assignment_is_capture = true;
							}
						}
						else if (is_event_capture_instruction(instruction_id))
						{
							const auto latest_instruction = get_latest_instruction();

							constexpr auto yield_index = util::variant_index<EntityInstruction::InstructionType, engine::instructions::Yield>();
							constexpr auto event_capture_index = util::variant_index<EntityInstruction::InstructionType, engine::instructions::EventCapture>();

							switch (latest_instruction->type_index())
							{
								case yield_index:
								case event_capture_index:
									if (assignment_expr_content.empty())
									{
										event_type_id = variable_description->type_id;

										variable_assignment_is_capture = true;
									}
									else
									{
										const auto opt_type_context = parsing_context.get_type_context();

										const auto event_type = (opt_type_context)
											? opt_type_context->get_type(assignment_expr_content)
											: resolve(hash(assignment_expr_content).value())
										;

										if (event_type)
										{
											event_type_id = event_type.id();

											variable_assignment_is_capture = true;
										}
										else
										{
											print_warn("Event capture failed: Unable to resolve event type.");
										}
									}

									break;

								default:
									print_warn("Event capture failed: Event capture instructions are only allowed immediately after a yield instruction. (Continuing as normal assignment)");

									break;
							}
						}

						if (variable_assignment_is_capture)
						{
							instruct<EventCapture>
							(
								MetaVariableTarget
								{
									variable_description->resolved_name,
									variable_description->scope
								},

								event_type_id
							);

							// Only one instruction processed, but several produced.
							return 1;
						}
					}
				}

				return multi_control_block
				(
					[this, &variable_name, &assignment_expr, &variable_type, &variable_description]() -> EntityInstructionCount
					{
						instruct<VariableDeclaration>
						(
							MetaVariableTarget
							{
								variable_description->resolved_name,
								variable_description->scope
							}
						);

						const auto full_assignment_expr_begin = variable_name.data();
						const auto full_assignment_expr_end = (assignment_expr.data() + assignment_expr.length());

						assert(full_assignment_expr_end > full_assignment_expr_begin);

						const auto full_assignment_expr_length = static_cast<std::size_t>(full_assignment_expr_end - full_assignment_expr_begin);
						const auto full_assignment_expr = std::string_view { full_assignment_expr_begin, full_assignment_expr_length };

						process_variable_assignment
						(
							variable_description->resolved_name, variable_description->scope,
							full_assignment_expr,
							variable_type,
							(variable_description->scope != MetaVariableScope::Local) // (variable_description->scope == MetaVariableScope::Global) // false
						);

						// Only one instruction processed, but several produced.
						return 1;
					}
				);
			}

			return 1;
		}

		return 0; // 1;
	}

	EntityInstructionCount EntityThreadBuilder::process_variable_assignment
	(
		MetaSymbolID resolved_variable_name,
		MetaVariableScope variable_scope,
		std::string_view full_assignment_expr,
		
		const MetaType& variable_type,

		bool ignore_if_already_assigned,
		bool ignore_if_not_declared
	)
	{
		using namespace engine::instructions;

		auto& storage = descriptor.get_shared_storage();

		auto assignment = meta_any_from_string
		(
			full_assignment_expr,

			{
				.context = parsing_context,
				.storage = &storage,

				.fallback_to_component_reference    = true,
				.fallback_to_entity_reference       = true,

				.allow_member_references            = true,
				.allow_entity_indirection           = true,
				.allow_remote_variable_references   = true

				//.allow_implicit_type_construction = true
			},

			variable_type
		);

		auto* as_indirect = assignment.try_cast<IndirectMetaAny>();

		if (!as_indirect)
		{
			return false;
		}

		bool underlying_has_indirection = type_has_indirection(as_indirect->get_type());

		assert(underlying_has_indirection);

		if (!underlying_has_indirection)
		{
			return false;
		}

		auto remote_instance = as_indirect->get(storage);

		if (!remote_instance)
		{
			return false;
		}

		return instruct_thread<VariableAssignment>
		(
			std::nullopt,
			std::move(*as_indirect),

			MetaVariableTarget { resolved_variable_name, variable_scope },

			ignore_if_already_assigned,
			ignore_if_not_declared
		);
	}

	std::optional<EntityInstructionCount> EntityThreadBuilder::process_directive_impl
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,

		const std::optional<EntityThreadInstruction>& thread_details,

		std::string_view instruction_raw,
		StringHash directive_id,
		std::string_view directive_content
	)
	{
		using namespace engine::instructions;
		using namespace engine::literals;

		switch (directive_id)
		{
			case "thread"_hs:
			case "begin"_hs:
				return generate_sub_thread(content_source, content_index, {}, true);

			case "end"_hs:
				// Stop processing.
				return std::nullopt;

			case "pause"_hs:
			case "sleep"_hs:
			case "wait"_hs:
			case "yield"_hs:
				// See also: `yield` command.
				return instruct_thread<Pause>(thread_details);

			case "multi"_hs:
			case "group"_hs:
			case "begin_multi"_hs:
				// See also: `multi` command.
				return generate_multi_block(content_source, content_index);

			case "fixed"_hs:
			case "begin_fixed"_hs:
				return generate_cadence_block(content_source, content_index, EntityThreadCadence::Fixed);

			case "realtime"_hs:
			case "begin_realtime"_hs:
				return generate_cadence_block(content_source, content_index, EntityThreadCadence::Realtime);

			//case "on_update"_hs:
			case "update_driven"_hs:
			case "begin_update_driven"_hs:
			case "frame_driven"_hs:
			case "begin_frame_driven"_hs:
				return generate_cadence_block(content_source, content_index, EntityThreadCadence::Update);

			//case "forever"_hs:
			case "repeat"_hs:
				return generate_repeat_block(content_source, content_index);

			case "stop"_hs:
			case "terminate"_hs:
				return instruct_thread<Stop>(thread_details);

			// Handles `scope`-prefix for variable declaration (e.g. `scope local`)
			case "scope"_hs:
				return process_variable_declaration(directive_content);

			case "var"_hs:
			case "auto"_hs:
			case "field"_hs:
			case "local"_hs:
			case "global"_hs:
			case "context"_hs:
			case "shared"_hs:
				return process_variable_declaration(instruction_raw);
		}

		return 0;
	}

	const EntityInstruction* EntityThreadBuilder::get_latest_instruction() const
	{
		auto& thread = get_thread();

		if (thread.empty())
		{
			return {};
		}

		const auto latest_instruction_index = (thread.size() - 1);

		return &(thread.get_instruction(latest_instruction_index));
	}

	EntityInstructionCount EntityThreadBuilder::launch(EntityThreadIndex thread_index)
	{
		return instruct<EntityThreadSpawnAction>
		(
			EntityThreadRange { thread_index, 1 },

			get_thread().name(),

			// Restart existing instance(s), if linked.
			true
		);
	}

	EntityInstructionCount EntityThreadBuilder::launch(const EntityThreadBuilderContext& context)
	{
		return launch(context.thread_index());
	}

	EntityInstructionCount EntityThreadBuilder::yield_thread_event
	(
		MetaTypeID event_type_id,

		const std::optional<EntityThreadInstruction>& event_thread_details,
		const std::optional<EntityThreadInstruction>& thread_details
	)
	{
		using namespace engine::literals;
		using namespace engine::instructions;

		auto fallback = [&]()
		{
			return instruct_thread<Yield>
			(
				thread_details,

				descriptor.allocate<EventTriggerCondition>
				(
					EventTriggerSingleCondition
					{
						MetaAny {}, // MetaAny { true },
						EventTriggerComparisonMethod::Equal,
						event_type_id
					}
				)
			);
		};

		if (!event_thread_details)
		{
			return fallback();
		}

		auto condition = EventTriggerAndCondition {};

		auto& storage = descriptor.get_shared_storage();

		condition.add_condition
		(
			descriptor.allocate<EventTriggerCondition>
			(
				EventTriggerSingleCondition
				{
					"entity"_hs,
					allocate_meta_any(event_thread_details->target_entity, &storage),
					EventTriggerComparisonMethod::Equal,
					event_type_id
				}
			)
		);

		if (event_thread_details->thread_id)
		{
			condition.add_condition
			(
				descriptor.allocate<EventTriggerCondition>
				(
					EventTriggerSingleCondition
					{
						"thread_id"_hs,
						allocate_meta_any(event_thread_details->thread_id, &storage),
						EventTriggerComparisonMethod::Equal,
						event_type_id
					}
				)
			);
		}

		return instruct_thread<Yield>
		(
			thread_details,

			descriptor.allocate<EventTriggerCondition>
			(
				std::move(condition)
			)
		);
	}

	EntityThreadBuilderContext EntityThreadBuilder::scope_context(MetaVariableContext& scope_local_variables) const
	{
		return EntityThreadBuilderContext
		{
			scope_context(),

			MetaParsingContext
			{
				get_type_context(),
				&scope_local_variables
			}
		};
	}

	const EntityThreadBuilderContext& EntityThreadBuilder::scope_context() const
	{
		return *(static_cast<const EntityThreadBuilderContext*>(this)); // *this;
	}

	MetaVariableContext EntityThreadBuilder::scope_variable_store(MetaSymbolID name)
	{
		return MetaVariableContext { get_variable_context(), name };
	}

	MetaVariableContext EntityThreadBuilder::scope_variable_store()
	{
		return scope_variable_store(static_cast<MetaSymbolID>(++scope_variable_context_count));
	}

	MetaVariableContext EntityThreadBuilder::sub_thread_variable_store(MetaSymbolID thread_id)
	{
		auto active_variable_context = parsing_context.get_variable_context();

		return MetaVariableContext
		{
			(active_variable_context)
				? active_variable_context->get_parent()
				: nullptr
			,

			thread_id
		};
	}

	MetaVariableContext EntityThreadBuilder::sub_thread_variable_store(std::string_view thread_name)
	{
		return sub_thread_variable_store
		(
			(thread_name.empty())
				? static_cast<MetaSymbolID>(descriptor.get_threads().size()) // MetaSymbolID {}
				: hash(thread_name).value()
		);
	}

	EntityThreadBuilderContext EntityThreadBuilder::sub_thread_context
	(
		ThreadDescriptor target_sub_thread,
		std::optional<MetaParsingContext> opt_parsing_context,
		std::optional<EntityThreadCadence> opt_cadence
	) const
	{
		return EntityThreadBuilderContext
		{
			descriptor,
			target_sub_thread,
			opt_factory_context,
			opt_base_path,

			(opt_parsing_context)
				? *opt_parsing_context
				: parsing_context,

			(opt_cadence)
				? *opt_cadence
				: cadence
		};
	}

	EntityThreadBuilderContext EntityThreadBuilder::sub_thread_context
	(
		std::optional<MetaParsingContext> opt_parsing_context,
		std::optional<EntityThreadCadence> opt_cadence
	) const
	{
		return sub_thread_context
		(
			ThreadDescriptor
			(
				descriptor,

				descriptor.shared_storage.allocate<EntityThreadDescription>(this->cadence)
			),

			opt_parsing_context,
			opt_cadence
		);
	}

	EntityThreadBuilder EntityThreadBuilder::sub_thread
	(
		std::string_view thread_name,
		
		std::optional<MetaParsingContext> opt_parsing_context,
		std::optional<EntityThreadCadence> opt_cadence
	)
	{
		return EntityThreadBuilder
		{
			sub_thread_context(opt_parsing_context, opt_cadence),
			thread_name
		};
	}

	EntityInstructionCount EntityThreadBuilder::generate_if_block
	(
		EventTriggerCondition&& condition,

		const ContentSource& content_source,
		EntityInstructionCount content_index
	)
	{
		using namespace engine::instructions;

		auto condition_ref = descriptor.allocate<EventTriggerCondition>(std::move(condition));

		auto& if_control_block = emit<IfControlBlock>
		(
			condition_ref,

			ControlBlock { 0 }
		);

		auto scope_local_variables = scope_variable_store();
		auto if_builder = EntityThreadIfBuilder { scope_context(scope_local_variables) };

		// NOTE: We add one to the initial content-index to
		// account for the `if` command we just processed.
		const auto if_index = (content_index + 1);

		const auto if_result = control_block
		(
			if_control_block.execution_range,

			[this, &content_source, if_index, &condition_ref, &if_builder]()
			{
				return if_builder.process(content_source, if_index);
			}
		);

		if (if_builder.has_else())
		{
			// Generate a `Skip` instruction to be reached during
			// a positive evaluation of the `if` condition.
			// (To be executed after the user-defined contents)
			auto& else_control_block = emit<Skip>
			(
				// Self target.
				EntityTarget {},

				// This thread. (No thread ID)
				EntityThreadID {},

				ControlBlock{ 0 }
			);

			// Include this `Skip` instruction in the bounds of the initial `if` control-block.
			if_control_block.execution_range.size += 1;

			// NOTE: The +2 scope-offset applied to `if_result` accounts for
			// both the initial `if` line as well as a trailing line.
			// 
			// This means the header of the `else` block we reached is already accounted for when 
			const auto else_index = (content_index + if_result);

			const auto else_result = control_block
			(
				else_control_block.instructions_skipped,

				[this, &content_source, else_index, &condition_ref, &if_builder]()
				{
					// TODO: Look into whether it still makes sense to re-use `if_builder` here,
					// since the variable context would also be shared.
					return if_builder.process(content_source, else_index);
				},
				
				// NOTE: Scope-offset set to one, rather than the usual two.
				// (Meant to account only for trailing `end` directive)
				1
			);

			// Progress past both the `if` and the `else` blocks.
			return (if_result + else_result);
		}

		// NOTE: The `control_block` subroutine handles result offsetting for both the
		// initial `if` directive and the trailing `end` directive.
		return if_result;
	}

	EntityInstructionCount EntityThreadBuilder::generate_while_block
	(
		EventTriggerCondition&& condition,

		const ContentSource& content_source,
		EntityInstructionCount content_index
	)
	{
		using namespace engine::instructions;

		auto condition_ref = descriptor.allocate<EventTriggerCondition>(std::move(condition));

		// Leading check for `condition`.
		auto& while_control_block = emit<IfControlBlock>
		(
			condition_ref,
			ControlBlock { 0 }
		);

		return control_block
		(
			while_control_block.execution_range,

			[this, &content_source, content_index, &condition_ref]()
			{
				const auto instruction_index = get_instruction_index();

				auto scope_local_variables = scope_variable_store();
				auto while_builder = EntityThreadWhileBuilder { scope_context(scope_local_variables) };
				
				const auto scope_result = while_builder.process(content_source, (content_index + 1));

				const auto updated_instruction_index = get_instruction_index();

				const auto scope_size =
				(
					static_cast<EntityInstructionCount>(updated_instruction_index)
					-
					static_cast<EntityInstructionCount>(instruction_index)
				);

				while_builder.instruct_thread<Rewind>
				(
					// Impact this thread.
					std::nullopt,

					// Rewind to the beginning of the `while` control block.
					// (+ 1 to retry the initial conditional check)
					static_cast<EntityInstructionCount>(scope_size + 1)
				);

				return scope_result;
			}
		);
	}

	EntityInstructionCount EntityThreadBuilder::generate_repeat_block
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index
	)
	{
		return generate_while_block
		(
			{ EventTriggerTrueCondition {} },

			content_source,
			content_index
		);
	}

	EntityInstructionCount EntityThreadBuilder::generate_when_block
	(
		EventTriggerCondition&& condition,

		const ContentSource& content_source,
		EntityInstructionCount content_index
	)
	{
		using namespace engine::instructions;

		auto when_variable_context = sub_thread_variable_store();

		// Start building a new (sub) thread for this `when` block.
		auto when_builder = sub_thread
		(
			{},

			MetaParsingContext
			{
				parsing_context.get_type_context(),
				&when_variable_context
			}
		);

		// Move the input-condition into a new location owned by the entity-descriptor.
		auto condition_ref = descriptor.allocate<EventTriggerCondition>(std::move(condition));

		// Yield on this sub-thread until this condition is met.
		when_builder.instruct_thread<Yield>
		(
			std::nullopt,

			// Reference to condition stored within descriptor.
			condition_ref
		);

		// NOTE: We offset by one here to account for the `when` instruction we're currently processing.
		const auto result = when_builder.process(content_source, (content_index + 1));

		if (result)
		{
			// On the main thread, emit a thread-spawn action so that
			// our sub-thread is launched when this point is reached.
			launch(when_builder);

			// NOTE: We add two here to account for the `when` instruction and
			// the corresponding `end` that `when_builder` reached before exiting.
			return (result + 2);
		}

		return result; // 0;
	}

	EntityInstructionCount EntityThreadBuilder::generate_sub_thread
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,

		std::string_view thread_name,
		bool launch_immediately
	)
	{
		using namespace engine::instructions;

		auto sub_thread_variable_context = sub_thread_variable_store(thread_name);

		// Start building a sub-thread until a corresponding `end` directive is reached.
		auto sub_thread_builder = sub_thread
		(
			thread_name,

			MetaParsingContext
			{
				parsing_context.get_type_context(),
				&sub_thread_variable_context
			}
		);

		// NOTE: We offset by one here to account for the `begin`/`thread` instruction we're currently processing.
		const auto result = sub_thread_builder.process(content_source, (content_index + 1));

		if (result)
		{
			if (launch_immediately)
			{
				// On the main thread, emit a thread-spawn action so that
				// our sub-thread is launched when this point is reached.
				launch(sub_thread_builder);
			}

			// NOTE: We add two here to account for the `begin`/`thread` instruction and
			// the corresponding `end` that `sub_thread_builder` reached before exiting.
			return (result + 2);
		}

		return result; // 0;
	}

	EntityInstructionCount EntityThreadBuilder::generate_sub_thread
	(
		const std::filesystem::path& path,
		std::string_view thread_name,
		bool launch_immediately
	)
	{
		auto imported_thread_variable_context = sub_thread_variable_store(thread_name);

		auto imported_thread = sub_thread
		(
			thread_name,

			MetaParsingContext
			{
				parsing_context.get_type_context(),
				&imported_thread_variable_context
			}
		);

		imported_thread.process_from_file(path);

		if (launch_immediately)
		{
			launch(imported_thread);
		}

		return 1;
	}

	EntityInstructionCount EntityThreadBuilder::generate_multi_block
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index
	)
	{
		using namespace engine::instructions;

		auto& multi_control_block = emit<MultiControlBlock>
		(
			ControlBlock { 0 }
		);

		return control_block
		(
			multi_control_block.included_instructions,

			[this, &content_source, content_index]()
			{
				auto scope_local_variables = scope_variable_store();
				auto multi_builder = EntityThreadMultiBuilder { scope_context(scope_local_variables) };
				
				return multi_builder.process(content_source, (content_index + 1));
			}
		);
	}

	EntityInstructionCount EntityThreadBuilder::generate_cadence_block
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,
		EntityThreadCadence cadence
	)
	{
		using namespace engine::instructions;

		auto& cadence_control_block = emit<CadenceControlBlock>
		(
			// Intended cadence for this control-block.
			cadence,

			ControlBlock { 0 }
		);

		return generate_cadence_block
		(
			content_source, content_index,
			cadence,
			cadence_control_block.included_instructions,
			true
		);
	}

	EntityInstructionCount EntityThreadBuilder::generate_cadence_block
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,
		EntityThreadCadence cadence,
		instructions::ControlBlock& control_block_out,
		bool generate_cadence_restore_instruction
	)
	{
		using namespace engine::instructions;

		const auto block_result = control_block
		(
			control_block_out,

			[this, &content_source, content_index, cadence]()
			{
				auto scope_local_variables = scope_variable_store();
				
				auto cadence_builder = EntityThreadCadenceBuilder
				{
					EntityThreadBuilderContext
					{
						scope_context(),

						// TODO: Look into whether we still want cadence to
						// act as a separate variable/parsing context.
						MetaParsingContext
						{
							get_type_context(),
							&scope_local_variables
						},
						
						cadence
					}
				};
				
				return cadence_builder.process(content_source, (content_index + 1));
			}
		);

		if (generate_cadence_restore_instruction)
		{
			emit<CadenceControlBlock>
			(
				this->cadence,

				ControlBlock { 0 }
			);
		}

		return block_result;
	}

	std::optional<EntityInstructionCount> EntityThreadBuilder::process_instruction
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,
		std::string_view instruction_raw
	)
	{
		using namespace engine::instructions;
		using namespace engine::literals;

		instruction_raw = util::trim(instruction_raw);

		if (instruction_raw.empty() || util::is_whitespace(instruction_raw))
		{
			// Skip this line, it's whitespace.
			return 1;
		}

		// TODO: Rework single-line comment implementation to account for strings.
		// 
		// Search for a single-line 'comment' symbol on this line:
		const auto single_line_comment_symbol_index = util::find_unscoped(instruction_raw, "//");

		switch (single_line_comment_symbol_index)
		{
			case 0:
				// This line starts with a comment-symbol, skip processing this line.
				return 1;
			
			case std::string_view::npos:
				// No comment symbol found, continue normally.
				break;

			default:
				// Truncate the current line at the comment-symbol we found.
				instruction_raw = instruction_raw.substr(0, single_line_comment_symbol_index);

				break;
		}

		auto [instruction, thread_details, thread_accessor_used] = parse_instruction_header(instruction_raw, &descriptor);

		//instruction = instruction.substr(parse_offset);

		//auto thread_instruction = process_thread_reference(descriptor, instruction_content);

		const auto
		[
			instruction_name,
			instruction_content,
			trailing_expr,
			is_string_content,
			instruction_parsed_length
		] = util::parse_command(instruction);

		auto error_impl = [content_index, &instruction_raw](bool continue_anyway, std::string_view message)
		{
			const auto line_number = (content_index + 1);

			auto error_message_out = util::format("Error on Line #{}: {}", line_number, message);

			print_error(error_message_out);
			print_error("#{}: {}", line_number, instruction_raw);

			if (continue_anyway)
			{
				print_warn("Continuing anyway...");
			}
			else
			{
				throw std::runtime_error(std::move(error_message_out));
			}
		};

		auto error = [&error_impl](std::string_view message)
		{
			error_impl(false, message);
		};

		auto warn = [&error_impl](std::string_view message)
		{
			error_impl(true, message);
		};

		bool try_as_meta_expr = true;

		if (instruction_name.empty())
		{
			auto
			[
				entity_ref_expr,
				type_or_variable_name, member_name,
				operator_symbol, assignment_value_raw,
				updated_offset
			] = parse_qualified_assignment_or_comparison(instruction_raw);

			bool meta_expr_allow_first_symbol_as_entity = false;
			bool meta_expr_allow_leading_remote_variable = true;

			bool try_as_directive                  = true;
			bool try_as_remote_variable_assignment = true;

			if (!type_or_variable_name.empty())
			{
				const auto type_or_variable_id = hash(type_or_variable_name).value();
				
				const auto& possible_thread_id = type_or_variable_id;

				if (descriptor.has_thread(possible_thread_id))
				{
					if ((!member_name.empty()) && (!assignment_value_raw.empty())) // && (!operator_symbol.empty())
					{
						meta_expr_allow_first_symbol_as_entity = false;
						meta_expr_allow_leading_remote_variable = false;

						try_as_directive = false;

						// `try_as_meta_expr` is currently left enabled as a formality.
						// (May change in the future)
						//try_as_meta_expr = false;
					}
				}
				else if (!thread_accessor_used)
				{
					const auto entity_target = (entity_ref_expr.empty())
						? EntityTarget { EntityTarget::SelfTarget {} }
						: EntityTarget::from_string(entity_ref_expr)
					;

					// Check for illegal `null` entity specification.
					if (entity_target.is_null_target())
					{
						error("Illegal usage of `null`");
					}
					else
					{
						// Try to process the raw instruction as an inline-update.
						const auto initial_update_instruction_result = process_update_instruction_from_values
						(
							type_or_variable_name,
							member_name,
							assignment_value_raw,
							operator_symbol,

							entity_target
						);

						if (initial_update_instruction_result)
						{
							// Update the previous-instruction ID to reflect
							// the inline-update operation we just processed.
							this->prev_instruction_id = "update"_hs;

							// No further processing needed.
							return 1;
						}
						
						/*
						if (!entity_ref_expr.empty())
						{
							meta_expr_allow_first_symbol_as_entity = true;
						}
						*/

						// This clause is a workaround for informal entity syntax.
						// i.e. this allows you to write `some_entity.some_property = some_value`,
						// rather than `entity(some_entity).some_property = some_value`.
						if (entity_ref_expr.empty()) // || entity_target.is_self_targeted()
						{
							bool leading_symbol_is_variable = false;

							// Make sure the symbol referenced isn't a variable:
							if (auto opt_variable_context = parsing_context.get_variable_context())
							{
								if (opt_variable_context->retrieve_variable(type_or_variable_name))
								{
									leading_symbol_is_variable = true;
								}
							}

							// NOTE: If this check was not present, any member-access from a variable could conflict with this logic.
							// e.g. `my_var.member` could be seen as a entity-component access, rather than a variable member access.
							if (!leading_symbol_is_variable)
							{
								auto member_as_type = MetaType {};
								auto member_id = MetaSymbolID {};

								const auto opt_type_context = get_type_context();

								if (opt_type_context)
								{
									member_as_type = opt_type_context->get_component_type(member_name);
								}
								else
								{
									member_id = hash(member_name);
									member_as_type = resolve(member_id);
								}

								if (member_as_type)
								{
									const auto& uncaptured_entity_ref = type_or_variable_name;

									const auto corrected_instruction_subset_offset = static_cast<std::size_t>((member_name.data() - instruction_raw.data()));
									const auto corrected_instruction_subset = instruction_raw.substr(corrected_instruction_subset_offset);

									auto
									[
										unlikely_nested_entity_ref_expr,
										intended_type_name, actual_member_name,
										intended_operator_symbol, intended_assignment_value_raw,
										intended_updated_offset
									] = parse_qualified_assignment_or_comparison(corrected_instruction_subset);

									if (!intended_type_name.empty() && !intended_operator_symbol.empty())
									{
										//assert(unlikely_nested_entity_ref_expr.empty());

										const auto corrected_update_instruction_result = process_update_instruction_from_values
										(
											intended_type_name,
											actual_member_name,
											intended_assignment_value_raw,
								
											(unlikely_nested_entity_ref_expr.empty())
												? uncaptured_entity_ref
												: unlikely_nested_entity_ref_expr
											,

											intended_operator_symbol
										);

										if (corrected_update_instruction_result)
										{
											// Update the previous-instruction ID to reflect
											// the inline-update operation we just processed.
											this->prev_instruction_id = "update"_hs;

											// No further processing needed.
											return 1;
										}
									}
								}
								else
								{
									// TODO: Look into whether removing this step makes sense.
									// (A similar validation step now happens in `meta_any_resolve_remote_variable_reference`)
									auto
									[
										unlikely_nested_entity_ref_expr,
										unqualified_entity_name, actual_member_name,
										intended_operator_symbol, intended_assignment_value_raw,
										intended_updated_offset
									] = parse_qualified_assignment_or_comparison(instruction_raw);

									if (!intended_operator_symbol.empty())
									{
										// Disable leading remote variables in favor of dedicated assignment logic.
										meta_expr_allow_leading_remote_variable = false;

										if (!unqualified_entity_name.empty() && !actual_member_name.empty())
										{
											if (const auto entity_type = resolve<Entity>())
											{
												if (!member_id)
												{
													member_id = hash(member_name);
												}

												if (entity_type.data(member_id))
												{
													// Found a valid data-member; treat this as an expression using an entity reference. (If applicable)
													meta_expr_allow_first_symbol_as_entity = true;

													// Don't bother processing this instruction as a directive or remote-variable assignment:
													try_as_directive = false;
													try_as_remote_variable_assignment = false;
												}
												else
												{
													const auto property_accessors = MetaProperty::generate_accessor_identifiers(member_name);
													const auto& setter_id = std::get<1>(property_accessors);

													if (auto setter_fn = entity_type.func(setter_id))
													{
														// Found a valid 'setter' function; treat this as a 'property' expression using an entity reference. (If applicable)
														meta_expr_allow_first_symbol_as_entity = true;

														// Don't bother processing this instruction as a directive or remote-variable assignment:
														try_as_directive = false;
														try_as_remote_variable_assignment = false;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}

			if (try_as_directive)
			{
				// Fallback to treating the 'headerless' instruction as a directive:
				auto directive_result = process_directive(content_source, content_index, thread_details, instruction);

				if (!directive_result.has_value() || (*directive_result > 0))
				{
					// No further processing needed.
					return directive_result;
				}
			}

			if (try_as_meta_expr)
			{
				if (auto result = process_meta_expression_instruction(instruction_raw, meta_expr_allow_first_symbol_as_entity, meta_expr_allow_leading_remote_variable))
				{
					return result;
				}
			}

			if (try_as_remote_variable_assignment && thread_details && !type_or_variable_name.empty()) // && (thread_details.target_entity.is_self_targeted() || thread_details.thread_id)
			{
				const auto& thread_name = type_or_variable_name;
				const auto& thread_local_variable_name = member_name;

				if (auto result = process_remote_variable_assignment(*thread_details, thread_name, thread_local_variable_name, assignment_value_raw, operator_symbol, entity_ref_expr))
				{
					return result;
				}
			}
			else
			{
				// All other resolution methods have been exhausted,
				// log that we weren't able to handle the instruction.
				warn("Failed to process instruction");

				return 1; // std::nullopt;
			}
		}
		else if (try_as_meta_expr)
		{
			if (auto result = process_meta_expression_instruction(instruction_raw, false, true))
			{
				return result;
			}
		}

		const auto instruction_id = hash(instruction_name).value();
		const auto prev_instruction_id = get_prev_instruction_id();

		// Handle continuous instructions:
		if (instruction_id != prev_instruction_id)
		{
			on_instruction_change(instruction_id, prev_instruction_id);
		}

		// TODO: Reimplement part of this routine to utilize `MetaParsingContext::instruction_aliases` automatically.
		auto handle_instruction = [&](auto instruction_id, std::string_view instruction_name) -> EntityInstructionCount // std::opitonal<EntityInstructionCount>
		{
			auto resolve_instruction = [this, &instruction_content]<typename InstructionType>
			(const std::optional<EntityThreadInstruction>& opt_thread_instruction)
			{
				return resolve_instruction_impl<InstructionType>(*this, opt_thread_instruction, instruction_content);
			};
			
			switch (instruction_id)
			{
				// NOTE: The `name` instruction only works for unnamed threads, and does not allow for dynamic input.
				case "name"_hs:
				{
					if (instruction_content.empty())
					{
						error("Unable to process `name` directive. (No name specified)");
					}
					else if (!set_thread_name(instruction_content, !is_string_content))
					{
						warn("Ignored `name` directive. (Thread is already named)");
					}

					return 1;
				}

				// NOTE: The content passed to this instruction does not allow for dynamic input.
				// The value may be left blank, if an anonymous thread is preferred.
				case "thread"_hs:
				case "begin"_hs:
					if (auto instruction_args = util::split_from<2>(instruction_content, ",", 0))
					{
						const auto& thread_name = std::get<0>(*instruction_args);
						const auto& launch_immediately_substr = std::get<1>(*instruction_args);

						const auto launch_immediately = util::from_string<bool>(launch_immediately_substr);

						return generate_sub_thread(content_source, content_index, thread_name, launch_immediately.value_or(true));
					}

					break;

				case "update"_hs:
				{
					// TODO: Revisit idea of comma-separated input for `update` instruction.
					// (Possible conflicts with inner expressions using this direct CSV method)
					/*
					// Check if the input was comma-separated vs. standard assignment:
					if (instruction_content.contains(','))
					{
						process_update_instruction_from_csv(instruction_content);

						return 1;
					}
					*/

					process_inline_update_instruction(instruction_content);

					return 1;
				}

				// Similar to a standard `when` block, but allows for generation of
				// multiple thread instances corresponding to each condition-block/event-type.
				case "when_each"_hs:
				{
					EntityInstructionCount instructions_processed = 0;

					// Process this sub-thread on each condition-block.
					process_trigger_expression
					(
						descriptor,

						instruction_content,

						[this, &content_source, content_index, &instructions_processed]
						(MetaTypeID type_id, std::optional<EventTriggerCondition> condition)
						{
							if (!condition)
							{
								return;
							}
							
							instructions_processed = std::max
							(
								instructions_processed,

								generate_when_block
								(
									std::move(*condition),
									content_source,
									content_index
								)
							);
						},

						false, // true,
						parsing_context
					);

					if (instructions_processed)
					{
						return instructions_processed;
					}
					else
					{
						error("Failed to generate `when_each` sub-thread(s).");
					}

					break;
				}

				// TODO: Add support for `else` within a `when` clause.
				case "when_any"_hs:
				case "when"_hs:
				{
					auto condition = process_unified_condition_block(descriptor, instruction_content, parsing_context);

					if (condition)
					{
						return generate_when_block(std::move(*condition), content_source, content_index);
					}
					else
					{
						error("Unable to resolve when-condition while building thread.");
					}

					break;
				}

				case "if"_hs:
				{
					if (auto condition = process_immediate_trigger_condition(instruction_content))
					{
						return generate_if_block(std::move(*condition), content_source, content_index);
					}
					else
					{
						error("Unable to resolve if-condition while building thread.");
					}

					break;
				}

				case "while"_hs:
				{
					if (auto condition = process_immediate_trigger_condition(instruction_content))
					{
						return generate_while_block(std::move(*condition), content_source, content_index);
					}
					else
					{
						error("Unable to resolve while-condition while building thread.");
					}

					break;
				}

				//case "forever"_hs:
				case "repeat"_hs:
					return generate_repeat_block(content_source, content_index);

				// Executes multiple instructions in one fixed update-step.
				// 
				// NOTE: Multi-instructions may be cut short if they contain a
				// control-flow instruction that suspends execution.
				// 
				// Nesting control-flow instructions within a multi-instruction
				// is generally considered bad practice and may have indeterminate results.
				case "multi"_hs:
				case "group"_hs:
				case "begin_multi"_hs:
					return generate_multi_block(content_source, content_index);

				// Cadence control-block instruction.
				case "cadence"_hs:
				case "begin_cadence"_hs:
				{
					if (instruction_content.empty())
					{
						error("Missing cadence specification.");
					}
					else
					{
						constexpr auto cadence_enum_prefix = std::string_view { "EntityThreadCadence" };

						const auto [leading_symbol, trailing_symbol, parsed_length] = util::parse_member_reference(instruction_content);

						auto cadence_value = std::string_view {};

						if (leading_symbol == cadence_enum_prefix)
						{
							cadence_value = trailing_symbol;
						}
						else
						{
							cadence_value = instruction_content;
						}

						if (!cadence_value.empty())
						{
							if (auto target_cadence = try_string_to_enum_value<EntityThreadCadence>(cadence_value))
							{
								if (auto cadence_block_count = generate_cadence_block(content_source, content_index, *target_cadence))
								{
									return cadence_block_count;
								}
							}
						}

						if (auto remote_instruction = resolve_instruction.operator()<CadenceControlBlock>(std::nullopt))
						{
							instruct<InstructionDescriptor>(std::move(*remote_instruction));

							auto cadence_block = ControlBlock { 0 };

							const auto block_result = generate_cadence_block
							(
								content_source, content_index,
								
								// NOTE: Transitive cadence is not possible with dynamic values.
								this->cadence,
								
								cadence_block,
								true
							);

							// TODO: Add block-size encoding support for dynamic cadence blocks. (`cadence_block`)

							return block_result;
						}
						else
						{
							error("Failed to resolve 'cadence' value.");
						}
					}

					break;
				}

				// Manual cadence assignment instruction.
				// (Unscoped / no control-block)
				case "set_cadence"_hs:
				{
					if (instruction_content.empty())
					{
						error("Missing cadence specification.");
					}
					else
					{
						constexpr auto cadence_enum_prefix = std::string_view { "EntityThreadCadence" };

						const auto [leading_symbol, trailing_symbol, parsed_length] = util::parse_member_reference(instruction_content);

						auto cadence_value = std::string_view {};

						if (leading_symbol == cadence_enum_prefix)
						{
							cadence_value = trailing_symbol;
						}
						else
						{
							cadence_value = instruction_content;
						}

						if (!cadence_value.empty())
						{
							if (auto target_cadence = try_string_to_enum_value<EntityThreadCadence>(cadence_value))
							{
								// Since this is a known cadence value, change our static cadence accordingly.
								this->cadence = *target_cadence;

								return instruct<CadenceControlBlock>
								(
									*target_cadence,

									ControlBlock { 0 }
								);
							}
						}

						if (auto remote_instruction = resolve_instruction.operator()<CadenceControlBlock>(std::nullopt))
						{
							// NOTE: Static cadence configuration cannot be updated using dynamic values.

							return instruct<InstructionDescriptor>(std::move(*remote_instruction));
						}
						else
						{
							error("Failed to resolve 'cadence' value.");
						}
					}

					break;
				}

				case "fixed"_hs:
				case "begin_fixed"_hs:
					return generate_cadence_block(content_source, content_index, EntityThreadCadence::Fixed);

				case "realtime"_hs:
				case "begin_realtime"_hs:
					return generate_cadence_block(content_source, content_index, EntityThreadCadence::Realtime);

				//case "on_update"_hs:
				case "update_driven"_hs:
				case "begin_update_driven"_hs:
				case "frame_driven"_hs:
				case "begin_frame_driven"_hs:
					return generate_cadence_block(content_source, content_index, EntityThreadCadence::Update);

				// NOTE: We don't currently handle the `check_linked` field for `Pause`.
				case "pause"_hs:
				{
					//const auto check_linked = util::from_string<bool>(instruction_content).value_or(true);

					//return instruct_thread<Pause>(thread_details, check_linked);
					return instruct_thread<Pause>(util::optional_or(std::get<0>(parse_thread_details(instruction_content)), thread_details));
				}

				case "wake"_hs:
				case "resume"_hs:
				{
					return instruct_thread<Resume>(util::optional_or(std::get<0>(parse_thread_details(instruction_content)), thread_details));
				}

				case "link"_hs:
					return instruct<Link>();

				// NOTE: Unlike `link`, the `unlink` instruction supports both regular and alternative thread-designation.
				// 
				// This is due to the default assumption that threads are linked. As a consequence,
				// re-linking a thread requires referencing local thread instances directly.
				case "unlink"_hs:
					return instruct_thread<Unlink>(util::optional_or(std::get<0>(parse_thread_details(instruction_content)), thread_details));

				// TODO: Determine if `attach` should take a state ID/name
				// or allow for alternate thread-designation syntax.
				// (`thread.attach()` vs. `attach(thread)`)
				case "attach"_hs:
				{
					if (!instruction_content.empty())
					{
						EntityStateID state_id = hash(instruction_content).value();

						return instruct_thread<Attach>(thread_details, state_id);
					}

					return instruct_thread<Attach>(thread_details);
				}

				// TODO: See notes for `attach`. For now, since `detach` never actually
				// handles state IDs, we assume the alternate thread-designation is fine.
				case "detach"_hs:
					//return instruct_thread<Detach>(thread_details);
					return instruct_thread<Detach>(util::optional_or(std::get<0>(parse_thread_details(instruction_content)), thread_details));

				// NOTE: For `start`, we currently allow both boolean input (restart behavior)
				// as well as the alternate thread-designation syntax.
				case "start"_hs:
				{
					std::optional<EntityThreadInstruction> target_thread_details = std::nullopt;

					bool restart_existing = false;
					bool yield_result = true;

					if (auto instruction_parse_result = util::split_from_ex<3>(instruction_content, ",", 1))
					{
						const auto& instruction_args = std::get<0>(*instruction_parse_result);

						auto restart_existing_as_first_parameter = util::from_string<bool>(instruction_args[0]);

						const std::size_t trailing_argument_offset = ((restart_existing_as_first_parameter) ? 1 : 2);

						target_thread_details = (restart_existing_as_first_parameter)
							? thread_details
							: util::optional_or(std::get<0>(parse_thread_details(instruction_args[0])), thread_details)
						;
						
						restart_existing = (restart_existing_as_first_parameter)
							? *restart_existing_as_first_parameter
							: util::from_string<bool>(instruction_args[1]).value_or(restart_existing)
						;

						yield_result = util::from_string<bool>(instruction_args[trailing_argument_offset]).value_or(true);
					}
					else
					{
						target_thread_details = thread_details;
					}

					// NOTE: This condition may be changed or removed at a later date.
					if (target_thread_details)
					{
						if (yield_result)
						{
							if (const auto thread_start_event_type = resolve<OnThreadSpawn>())
							{
								return multi_control_block
								(
									[&]()
									{
										instruct_thread<Start>
										(
											target_thread_details,
											restart_existing
										);

										yield_thread_event(thread_start_event_type.id(), target_thread_details, thread_details);

										// Multiple instructions generated, but only one item processed. (i.e. `start`)
										return 1;
									}
								);
							}
						}
					}

					instruct_thread<Start>
					(
						target_thread_details,
						restart_existing
					);

					return 1;
				}

				case "restart"_hs:
				{
					if (auto instruction_parse_result = util::split_from_ex<2>(instruction_content, ",", 0))
					{
						const auto& instruction_args = std::get<0>(*instruction_parse_result);

						auto yield_result_as_first_parameter = util::from_string<bool>(instruction_args[0]);

						auto target_thread_details = (yield_result_as_first_parameter)
							? thread_details
							: util::optional_or(std::get<0>(parse_thread_details(instruction_args[0])), thread_details)
						;

						// NOTE: This condition may be changed or removed at a later date.
						if (target_thread_details)
						{
							const auto yield_result = (yield_result_as_first_parameter)
								? *yield_result_as_first_parameter
								: util::from_string<bool>(instruction_args[1]).value_or(true)
							;

							if (yield_result)
							{
								if (const auto thread_start_event_type = resolve<OnThreadSpawn>())
								{
									return multi_control_block
									(
										[&]()
										{
											instruct_thread<Restart>(target_thread_details);

											yield_thread_event(thread_start_event_type.id(), target_thread_details, thread_details);

											// Multiple instructions generated, but only one item processed. (i.e. `restart`)
											return 1;
										}
									);
								}
							}
						}

						return instruct_thread<Restart>(target_thread_details);
					}

					return instruct_thread<Restart>(thread_details);
				}

				// NOTE: We don't currently handle the `check_linked` field for `Stop`.
				case "terminate"_hs:
				case "stop"_hs:
				{
					//const auto check_linked = util::from_string<bool>(instruction_content).value_or(true);

					//return instruct_thread<Stop>(thread_details, check_linked);
					return instruct_thread<Stop>(util::optional_or(std::get<0>(parse_thread_details(instruction_content)), thread_details));
				}

				case "assert"_hs:
				{
					if (instruction_content.empty())
					{
						error("Unable to generate assert instruction; condition missing.");
					}
					else
					{
						if (auto instruction_args = util::split_from<2>(instruction_content, ",", 1))
						{
							const auto& condition_content = (*instruction_args)[0];

							assert(!condition_content.empty());

							if (auto condition = process_immediate_trigger_condition(condition_content))
							{
								auto condition_ref = descriptor.allocate<EventTriggerCondition>(std::move(*condition));

								using remote_message_t = decltype(instructions::Assert::debug_message);

								remote_message_t debug_message;

								if (auto& debug_message_raw = (*instruction_args)[1]; !debug_message_raw.empty())
								{
									debug_message = descriptor.allocate<std::string>(util::unquote_safe(debug_message_raw));
								}

								return instruct<Assert>
								(
									std::move(condition_ref),
									std::move(debug_message),

									descriptor.allocate<std::string>(condition_content)
								);
							}
							else
							{
								error("Unable to resolve assert-condition while building thread.");
							}
						}
						else
						{
							error("Unable to resolve assert-condition while building thread.");
						}
					}

					break;
				}

				// NOTE:
				// `sleep`, `wait` and `yield` have the same capabilities, but different prioritization for inputs.
				// e.g. `sleep` will look for a duration first, whereas `yield` will look for a trigger condition first.
				// 
				// This allows `sleep` to take a variable as input for its duration, whereas
				// `wait` and `yield` would assume the variable is a wake condition.
				// 
				// See also: `wait`, `yield`
				case "sleep"_hs:
				{
					if (instruction_content.empty())
					{
						return instruct_thread<Pause>(thread_details);
					}
					else
					{
						if (auto sleep_duration = parse_time_duration(instruction_content))
						{
							return instruct_thread<Sleep>(thread_details, std::move(*sleep_duration));
						}
						else if (auto remote_instruction = resolve_instruction.operator()<Sleep>(thread_details))
						{
							return instruct<InstructionDescriptor>(std::move(*remote_instruction));
						}
						else if (auto condition = process_unified_condition_block(descriptor, instruction_content, parsing_context))
						{
							auto condition_ref = descriptor.allocate<EventTriggerCondition>(std::move(*condition));

							return instruct_thread<Yield>(thread_details, std::move(condition_ref));
						}
						else
						{
							error("Failed to resolve `yield` expression.");
						}
					}

					break;
				}

				// See also: `sleep`, `yield`
				case "wait"_hs:
				{
					if (instruction_content.empty())
					{
						return instruct_thread<Pause>(thread_details);
					}
					else
					{
						if (auto sleep_duration = parse_time_duration(instruction_content))
						{
							return instruct_thread<Sleep>(thread_details, std::move(*sleep_duration));
						}
						else if (auto condition = process_unified_condition_block(descriptor, instruction_content, parsing_context, true))
						{
							auto condition_ref = descriptor.allocate<EventTriggerCondition>(std::move(*condition));

							return instruct_thread<Yield>(thread_details, std::move(condition_ref));
						}
						else if (auto remote_instruction = resolve_instruction.operator()<Sleep>(thread_details))
						{
							return instruct<InstructionDescriptor>(std::move(*remote_instruction));
						}
						else
						{
							error("Failed to resolve `yield` expression.");
						}
					}

					break;
				}

				// See also: `sleep`, `wait`
				case "yield"_hs:
				{
					if (instruction_content.empty())
					{
						return instruct_thread<Pause>(thread_details);
					}
					else
					{
						if (auto sleep_duration = parse_time_duration(instruction_content))
						{
							return instruct_thread<Sleep>(thread_details, std::move(*sleep_duration));
						}
						else if (auto condition = process_unified_condition_block(descriptor, instruction_content, parsing_context))
						{
							auto condition_ref = descriptor.allocate<EventTriggerCondition>(std::move(*condition));

							return instruct_thread<Yield>(thread_details, std::move(condition_ref));
						}
						else if (auto remote_instruction = resolve_instruction.operator()<Sleep>(thread_details))
						{
							return instruct<InstructionDescriptor>(std::move(*remote_instruction));
						}
						else
						{
							error("Failed to resolve `yield` expression.");
						}
					}

					break;
				}

				case "skip"_hs:
				case "step"_hs:
				{
					if (const auto instructions_skipped = util::from_string<EntityInstructionCount>(instruction_content))
					{
						assert(*instructions_skipped > 0);

						return instruct_thread<Skip>(thread_details, *instructions_skipped);
					}

					//return instruct_thread<Step>(thread_details);

					break;
				}

				case "merge"_hs:
				case "include"_hs:
					// NOTE: Direct use of `from_file`.
					return from_file(instruction_content);

				case "embed"_hs:
					// NOTE: Direct use of `from_lines`.
					return from_lines(instruction_content);

				case "import"_hs:
					return generate_sub_thread(std::filesystem::path { instruction_content });

				default:
				{
					// If this instruction ID couldn't be identified, assume it's a command instruction.
					return process_command_instruction
					(
						instruction_id, instruction_name,
						instruction_content,
						
						!is_string_content
					);
				}
			}

			return 0;
		};

		if (const auto instructions_processed = handle_instruction(instruction_id, instruction_name))
		{
			return instructions_processed;
		}
		else
		{
			// Fallback to account for case-sensitivity:
			const auto lowercase_instruction_name = util::lowercase(instruction_name);
			const auto lowercase_instruction_id = hash(lowercase_instruction_name).value();

			if (const auto instructions_processed = handle_instruction(lowercase_instruction_id, lowercase_instruction_name))
			{
				return instructions_processed;
			}
		}

		// Try processing as a meta-expression again, but with entity-reference fallbacks turned on.
		if (try_as_meta_expr)
		{
			if (auto result = process_meta_expression_instruction(instruction_raw, true, true)) // false
			{
				return result;
			}
		}

		// If all else fails, notify the user and continue processing.
		warn(util::format("Unknown instruction detected: `{}` (#{})", instruction_name, instruction_id));

		// NOTE: We always return 1 on invalid instructions, since
		// we don't (currently) stop processing when this happens.
		return 1;
	}

	EntityInstructionCount EntityThreadBuilder::from_array(const util::json& thread_content, EntityInstructionCount skip)
	{
		using namespace engine::instructions;

		EntityInstructionCount processed_instruction_count = 0;

		// TODO: Optimize initial instruction skipping.
		EntityInstructionCount active_skip_count = skip;

		EntityInstructionCount content_index = 0;

		util::json_for_each
		(
			thread_content,

			[&](const util::json& instruction_data)
			{
				if (active_skip_count > 0)
				{
					active_skip_count--;
					content_index++;

					return true;
				}

				switch (instruction_data.type())
				{
					case util::json::value_t::string:
					{
						const auto instruction = instruction_data.get<std::string>();

						const auto result = process_instruction(std::reference_wrapper<const util::json>(thread_content), content_index, instruction);

						if (!result.has_value())
						{
							return false;
						}

						const auto& processed = *result;

						if (processed > 1)
						{
							// NOTE: When skipping, we ignore the number of instructions processed minus one,
							// since the value of `processed` is inclusive of the instruction we began at.
							active_skip_count += (processed - 1);
						}

						processed_instruction_count += processed;

						break;
					}

					case util::json::value_t::object:
					{
						if (on_update_instruction(instruction_data))
						{
							processed_instruction_count++;
						}

						break;
					}

					case util::json::value_t::array:
					{
						auto& multi_control_block = emit<MultiControlBlock>
						(
							ControlBlock { 0 }
						);

						processed_instruction_count += control_block
						(
							multi_control_block.included_instructions,

							[this, &instruction_data]()
							{
								// NOTE: Recursion.
								return from_array(instruction_data);
							},

							// No need for a scope-offset, since this
							// is a JSON array, rather than a regular scope.
							0
						);

						break;
					}
				}
				
				content_index++;

				return true;
			}
		);

		return processed_instruction_count;
	}

	EntityInstructionCount EntityThreadBuilder::from_object(const util::json& thread_content, bool flush)
	{
		on_update_instruction(thread_content);

		if (flush)
		{
			flush_update_instruction();
		}

		return 1;
	}

	EntityInstructionCount EntityThreadBuilder::from_lines(std::string_view lines, std::string_view separator, EntityInstructionCount skip)
	{
		auto content_index = static_cast<EntityInstructionCount>(0);
		auto processed_instruction_count = EntityInstructionCount {};
		auto active_skip_count = skip;

		util::split
		(
			lines, separator,

			[this, &lines, &content_index, &active_skip_count, &processed_instruction_count](std::string_view instruction, bool is_last_instruction=false)
			{
				if (active_skip_count > 0)
				{
					active_skip_count--;
					content_index++;

					return true;
				}

				if (instruction.empty())
				{
					processed_instruction_count++;
				}
				else
				{
					const auto result = process_instruction(lines, content_index, instruction);
				
					if (!result.has_value())
					{
						return false;
					}

					const auto& processed = *result;

					if (processed > 1)
					{
						// NOTE: When skipping, we ignore the number of instructions processed minus one,
						// since the value of `processed` is inclusive of the instruction we began at.
						active_skip_count += (processed - 1);
					}

					processed_instruction_count += processed;
				}

				content_index++;

				return true;
			},

			false, // separator_required

			util::whitespace_symbols, // trim_values

			true // include_empty_substrings
		);

		return processed_instruction_count;
	}

	EntityInstructionCount EntityThreadBuilder::from_precompiled_script(PrecompiledScriptID script_id)
	{
		using namespace engine::instructions;

		if (const auto global_script_type = resolve<engine::ScriptNamespace>())
		{
			const auto& thread = get_thread();

			const auto initial_instruction_count = thread.instructions.size();

			auto& storage = descriptor.get_shared_storage();

			auto& script_function_call = storage.allocate<MetaFunctionCall>
			(
				(global_script_type)
					? global_script_type.id()
					: MetaTypeID {}
				,

				script_id
			);

			const bool already_realtime = (this->cadence == EntityThreadCadence::Realtime);

			if (!already_realtime)
			{
				// Use realtime cadence for scripts.
				emit<ChangeCadence>(EntityThreadCadence::Realtime);
			}

			instruct<CoroutineCall>(IndirectMetaAny { storage, script_function_call });

			if (!already_realtime)
			{
				// Restore standard cadence for subsequent instructions.
				emit<ChangeCadence>(this->cadence);
			}

			const auto current_instruction_count = thread.instructions.size();

			return static_cast<EntityInstructionCount>(current_instruction_count - initial_instruction_count);
		}

		return {};
	}

	EntityInstructionCount EntityThreadBuilder::from_file(const std::filesystem::path& script_path, std::string_view separator, EntityInstructionCount skip)
	{
		auto instructions_processed = EntityInstructionCount {};

		auto resolved_cpp_path = std::optional<std::pair<std::filesystem::path, PrecompiledScriptID>> {};

		if (opt_factory_context)
		{
			resolved_cpp_path = opt_factory_context->resolve_cpp_script_reference_ex
			(
				script_path,

				(
					(opt_base_path)
					? *opt_base_path
					: std::filesystem::path {}
				)
			);
		}
		else
		{
#if (GLARE_SCRIPT_PRECOMPILED)
			resolved_cpp_path = { { script_path, engine::hash(script_path.string()) } };
#else
			resolved_cpp_path = { { script_path, PrecompiledScriptID {} } };
#endif
		}

#if (GLARE_SCRIPT_PRECOMPILED)
		if (resolved_cpp_path)
		{
			const auto& [precompiled_script_path, precompiled_script_id] = *resolved_cpp_path;

			if (!thread_has_name())
			{
				const auto thread_name = thread_name_from_resolved_path(precompiled_script_path);

				set_thread_name(thread_name);
			}

			instructions_processed += from_precompiled_script(precompiled_script_id);
		}
#else
		// TODO: Implement Cling/Clang-REPL support.
		static_assert(false, "Pre-compilation currently required for CPP script integration.")

		//if (std::filesystem::exists(script_path)) ...
#endif

		auto resolved_es_path = std::filesystem::path {};

		if (opt_factory_context)
		{
			resolved_es_path = opt_factory_context->resolve_entity_script_reference
			(
				script_path,

				(opt_base_path)
					? *opt_base_path
					: std::filesystem::path {}
			);
		}
		else
		{
			resolved_es_path = script_path;
		}

		if (std::filesystem::exists(resolved_es_path))
		{
			const auto es_script_data = util::load_string(resolved_es_path.string());

			if (!es_script_data.empty())
			{
				if (!thread_has_name())
				{
					const auto thread_name = thread_name_from_resolved_path(resolved_es_path);

					set_thread_name(thread_name);
				}

				instructions_processed += from_lines(es_script_data, separator, skip); // process(std::string_view(es_script_data), skip);
			}
		}

		return instructions_processed;
	}

	EntityInstructionCount EntityThreadBuilder::from_file(const std::string& script_path_str, std::string_view separator, EntityInstructionCount skip)
	{
		return from_file(std::filesystem::path { script_path_str }, separator, skip);
	}

	EntityInstructionCount EntityThreadBuilder::from_file(std::string_view script_path_str, std::string_view separator, EntityInstructionCount skip)
	{
		return from_file(std::string { script_path_str }, separator, skip);
	}

	EntityInstructionCount EntityThreadBuilder::process(const util::json& content, EntityInstructionCount skip)
	{
		EntityInstructionCount processed_instruction_count = 0;

		switch (content.type())
		{
			// Path to dedicated script file.
			case util::json::value_t::string:
			{
				const auto script_path = content.get<std::string>();

				processed_instruction_count += from_file(std::filesystem::path(script_path), "\n", skip);

				break;
			}
			
			// Single update instruction acting as thread contents.
			case util::json::value_t::object:
			{
				processed_instruction_count += from_object(content, false);

				break;
			}

			// Standard array-based thread description.
			case util::json::value_t::array:
			{
				processed_instruction_count += from_array(content, skip);

				break;
			}
		}

		// Ensure to flush if we're in the middle of
		// processing a multi-line update instruction.
		flush_update_instruction();

		return processed_instruction_count;
	}

	EntityInstructionCount EntityThreadBuilder::process(std::string_view lines, EntityInstructionCount skip)
	{
		const auto result = from_lines(lines, "\n", skip);

		// Ensure to flush the current update instruction.
		flush_update_instruction();

		return result;
	}

	EntityInstructionCount EntityThreadBuilder::process(const ContentSource& content, EntityInstructionCount skip)
	{
		EntityInstructionCount result = 0;

		util::visit
		(
			content,

			[this, skip, &result](std::string_view lines)
			{
				result = process(lines, skip);
			},

			[this, skip, &result](const util::json& data)
			{
				result = process(data, skip);
			}
		);

		// NOTE: There's no need to call `flush_update_instruction` here since this is a delegating overload.

		return result;
	}

	// EntityThreadIfBuilder:
	EntityThreadIfBuilder::EntityThreadIfBuilder
	(
		const EntityThreadBuilderContext& context
	) :
		EntityThreadBuilder(context)
	{}

	std::optional<EntityInstructionCount> EntityThreadIfBuilder::process_directive_impl
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,

		const std::optional<EntityThreadInstruction>& thread_details,

		std::string_view instruction_raw,
		StringHash directive_id,
		std::string_view directive_content
	)
	{
		using namespace engine::literals;

		switch (directive_id)
		{
			case "endif"_hs:
			{
				return std::nullopt;
			}
			case "else"_hs:
			{
				exited_on_else = true;

				return std::nullopt;
			}
		}

		return EntityThreadBuilder::process_directive_impl
		(
			content_source,
			content_index,

			thread_details,

			instruction_raw,
			directive_id,
			directive_content
		);
	}

	// EntityThreadWhileBuilder:
	EntityThreadWhileBuilder::EntityThreadWhileBuilder
	(
		const EntityThreadBuilderContext& context
	) :
		EntityThreadBuilder(context)
	{}

	std::optional<EntityInstructionCount> EntityThreadWhileBuilder::process_directive_impl
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,

		const std::optional<EntityThreadInstruction>& thread_details,

		std::string_view instruction_raw,
		StringHash directive_id,
		std::string_view directive_content
	)
	{
		using namespace engine::literals;

		switch (directive_id)
		{
			case "endwhile"_hs:
			case "end_while"_hs:
			{
				return std::nullopt;
			}
		}

		return EntityThreadBuilder::process_directive_impl
		(
			content_source,
			content_index,

			thread_details,

			instruction_raw,
			directive_id,
			directive_content
		);
	}

	// EntityThreadMultiBuilder:
	std::optional<EntityInstructionCount> EntityThreadMultiBuilder::process_directive_impl
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,

		const std::optional<EntityThreadInstruction>& thread_details,

		std::string_view instruction_raw,
		StringHash directive_id,
		std::string_view directive_content
	)
	{
		using namespace engine::literals;

		switch (directive_id)
		{
			case "end_multi"_hs:
			case "end_group"_hs:
				return std::nullopt;
		}

		return EntityThreadBuilder::process_directive_impl
		(
			content_source,
			content_index,

			thread_details,

			instruction_raw,
			directive_id,
			directive_content
		);
	}

	// EntityThreadCadenceBuilder:
	std::optional<EntityInstructionCount> EntityThreadCadenceBuilder::process_directive_impl
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,

		const std::optional<EntityThreadInstruction>& thread_details,

		std::string_view instruction_raw,
		StringHash directive_id,
		std::string_view directive_content
	)
	{
		using namespace engine::literals;

		switch (directive_id)
		{
			case "end_cadence"_hs:
			case "end_fixed"_hs:
			case "end_realtime"_hs:
			case "end_frame_driven"_hs:
				return std::nullopt;
		}

		return EntityThreadBuilder::process_directive_impl
		(
			content_source,
			content_index,

			thread_details,

			instruction_raw,
			directive_id,
			directive_content
		);
	}
}