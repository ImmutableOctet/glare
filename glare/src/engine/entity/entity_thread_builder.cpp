#include "entity_thread_builder.hpp"
#include "entity_thread_description.hpp"
#include "entity_factory_context.hpp"
#include "entity_descriptor.hpp"

#include "serial.hpp"
#include "serial_impl.hpp"

#include <engine/timer.hpp>

#include <engine/meta/meta.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/parsing_context.hpp>
#include <engine/meta/meta_type_descriptor.hpp>

#include <tuple>

#include <util/string.hpp>
#include <util/parse.hpp>
#include <util/io.hpp>
#include <util/optional.hpp>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	// EntityThreadBuilderContext:
	EntityThreadBuilderContext::EntityThreadBuilderContext
	(
		EntityDescriptor& descriptor,
		EntityThreadDescription& thread,
		const EntityFactoryContext* opt_factory_context,
		const std::filesystem::path* opt_base_path,
		const ParsingContext* opt_parsing_context
	) :
		descriptor(descriptor),
		thread(thread),
		opt_factory_context(opt_factory_context),
		opt_base_path(opt_base_path),
		opt_parsing_context(opt_parsing_context)
	{}

	EntityThreadIndex EntityThreadBuilderContext::thread_index() const
	{
		return descriptor.shared_storage.get_index_safe(thread);
	}

	// EntityThreadBuilder:
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
		EntityThreadDescription& thread,
		std::string_view opt_thread_name,
		const EntityFactoryContext* opt_factory_context,
		const std::filesystem::path* opt_base_path,
		const ParsingContext* opt_parsing_context
	)
		: EntityThreadBuilder
		(
			EntityThreadBuilderContext
			(
				descriptor,
				thread,
				opt_factory_context,
				opt_base_path,
				opt_parsing_context
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
		const ParsingContext* opt_parsing_context
	)
		: EntityThreadBuilder
		(
			descriptor,
			descriptor.shared_storage.allocate<EntityThreadDescription>(),
			opt_thread_name,
			opt_factory_context,
			opt_base_path,
			opt_parsing_context
		)
	{}

	EntityThreadBuilder::~EntityThreadBuilder() {}

	std::optional<EntityThreadID> EntityThreadBuilder::set_thread_name(std::string_view thread_name, bool resolve_name, bool force)
	{
		if ((!force) && (thread.thread_id))
		{
			return std::nullopt; // thread.thread_id;
		}

		if (thread_name.empty())
		{
			return std::nullopt;
		}

		std::optional<EntityThreadID> thread_id_out = std::nullopt;

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
					.strip_quotes = false,

					// Allows fallthrough to non-resolution path below.
					.fallback_to_string = false,

					// We determine thread names during the initial processing phase,
					// and will therefore have no use for runtime name resolution.
					.resolve_component_member_references = false
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
			print_warn("Attempted to set thread ID to #{}, but an existing thread already has that name. (Input: \"{}\")", *thread_id_out, thread_name);
		}
		//else
		{
			thread.thread_id = thread_id_out;
		}

		//return thread_id_out;
		return thread.thread_id;
	}

	std::optional<EntityThreadID> EntityThreadBuilder::get_thread_name() const
	{
		return thread.thread_id;
	}

	bool EntityThreadBuilder::thread_has_name() const
	{
		return static_cast<bool>(get_thread_name());
	}

	EntityThreadBuilder::InstructionIndex EntityThreadBuilder::get_instruction_index()
	{
		return static_cast<InstructionIndex>(thread.instructions.size());
	};

	EntityStateUpdateAction& EntityThreadBuilder::get_update_instruction(EntityTarget target)
	{
		// Check if we're already building an update instruction:
		if (current_update_instruction)
		{
			// Determine if the target entity is different:
			if (current_update_instruction->target_entity == target)
			{
				return *current_update_instruction;
			}
			
			// Flush the existing update instruction,
			// since we're updating a different entity.
			flush_update_instruction();
		}

		current_update_instruction = EntityStateUpdateAction
		{
			std::make_unique<EntityStateUpdateAction::ComponentStore>(),
			target
		};

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

		return (process_update_action(update_instruction, update_entry, opt_parsing_context) > 0);
	}

	bool EntityThreadBuilder::on_instruction_change(InstructionID instruction_id, InstructionID prev_instruction_id)
	{
		using namespace entt::literals;

		this->prev_instruction_id = instruction_id;

		switch (prev_instruction_id)
		{
			case "update"_hs:
				return static_cast<bool>(flush_update_instruction());
		}

		return false;
	};

	MetaTypeDescriptor* EntityThreadBuilder::process_update_instruction_from_values
	(
		std::string_view type_name,
		std::string_view member_name,
		std::string_view value_raw,
		std::string_view entity_ref_expr
	)
	{
		if (type_name.empty())
		{
			//print_error("Missing type-name detected.");

			return {};
		}

		auto target = EntityTarget::parse(entity_ref_expr);

		if (!target)
		{
			target = { EntityTarget::SelfTarget {} };
		}

		// Retrieve the current update instruction, or
		// initialize a new one if one doesn't exist yet.
		auto& update_instruction = get_update_instruction(*target);

		auto type_id = hash(type_name);
		auto type = resolve(type_id);

		if (!type)
		{
			print_error("Unable to resolve type: {} (#{})", type_name, type_id.value());

			return {};
		}

		auto* target_descriptor = update_instruction.updated_components->get_definition(type_id);

		if (!target_descriptor)
		{
			target_descriptor = &(update_instruction.updated_components->type_definitions.emplace_back(type));
		}

		target_descriptor->set_variable(MetaVariable { member_name, meta_any_from_string(value_raw) });

		return target_descriptor;
	}

	MetaTypeDescriptor* EntityThreadBuilder::process_update_instruction_from_csv(std::string_view instruction_separated, std::string_view separator)
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

	MetaTypeDescriptor* EntityThreadBuilder::process_inline_update_instruction(std::string_view instruction)
	{
		auto
		[
			entity_ref_expr,
			type_name, member_name,
			operator_symbol, value_raw,
			updated_offset
		] = util::parse_qualified_assignment_or_comparison(instruction);

		return process_update_instruction_from_values(type_name, member_name, value_raw, entity_ref_expr);
	};

	EntityInstructionCount EntityThreadBuilder::process_command_instruction
	(
		InstructionID instruction_id,
		std::string_view instruction_name,
		std::string_view instruction_content,
		bool resolve_values,
		std::string_view separator
	)
	{
		auto command_type = resolve(instruction_id);

		if (!command_type)
		{
			if (opt_parsing_context)
			{
				command_type = opt_parsing_context->get_command_type_from_alias(instruction_name);

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
				.resolve_symbol=resolve_values,
				.resolve_component_member_references=resolve_values
			},
			
			separator, command_arg_offset
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
		auto directive_id = hash(directive).value();

		return process_directive_impl
		(
			content_source,
			content_index,

			thread_details,

			directive_id
		);
	}

	std::optional<EntityInstructionCount> EntityThreadBuilder::process_directive_impl
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,

		const std::optional<EntityThreadInstruction>& thread_details,

		StringHash directive_id
	)
	{
		using namespace engine::instructions;
		using namespace entt::literals;

		switch (directive_id)
		{
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

			//case "forever"_hs:
			case "repeat"_hs:
				return generate_repeat_block(content_source, content_index);

			case "stop"_hs:
			case "terminate"_hs:
				return instruct_thread<Stop>(thread_details);
		}

		return 0;
	}

	EntityInstructionCount EntityThreadBuilder::launch(EntityThreadIndex thread_index)
	{
		return instruct<EntityThreadSpawnAction>
		(
			EntityThreadRange { thread_index, 1 },

			// Restart existing instance(s), if linked.
			true
		);
	}

	EntityInstructionCount EntityThreadBuilder::launch(const EntityThreadBuilderContext& context)
	{
		return launch(context.thread_index());
	}

	const EntityThreadBuilderContext& EntityThreadBuilder::context() const
	{
		return *this;
	}

	EntityThreadBuilderContext EntityThreadBuilder::sub_context(EntityThreadDescription& target_sub_thread) const
	{
		return EntityThreadBuilderContext
		{
			descriptor,
			target_sub_thread,
			opt_factory_context,
			opt_base_path,
			opt_parsing_context
		};
	}

	EntityThreadBuilderContext EntityThreadBuilder::sub_context() const
	{
		return sub_context(descriptor.shared_storage.allocate<EntityThreadDescription>());
	}

	EntityThreadBuilder EntityThreadBuilder::sub_thread(std::string_view thread_name)
	{
		return EntityThreadBuilder
		{
			sub_context(),
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

		auto if_builder = EntityThreadIfBuilder { context() };

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
				std::optional<EntityThreadID>(std::nullopt),

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

				auto while_builder = EntityThreadWhileBuilder { context() };
				
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
			EventTriggerTrueCondition {},

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

		// Start building a new (sub) thread for this `when` block.
		auto when_builder = sub_thread();

		// Move the input-condition into a new location owned by the entity-descriptor.
		auto condition_ref = descriptor.allocate<EventTriggerCondition>(std::move(condition));

		// Yield on this sub-thread until this condition is met.
		when_builder.instruct_thread<Yield>
		(
			std::nullopt,

			// Reference to condition stored within descriptor.
			condition_ref
		);

		const auto result = when_builder.process(content_source, (content_index + 1));

		// NOTE: We offset by one here to account for the `when` instruction we're currently processing.
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
				auto multi_builder = EntityThreadMultiBuilder { context() };
				
				return multi_builder.process(content_source, (content_index + 1));
			}
		);
	}

	std::optional<EntityInstructionCount> EntityThreadBuilder::process_instruction
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,
		std::string_view instruction_raw
	)
	{
		using namespace engine::instructions;
		using namespace entt::literals;

		auto [instruction, thread_details] = parse_instruction_header(instruction_raw, &descriptor);

		//instruction = instruction.substr(parse_offset);

		//auto thread_instruction = process_thread_reference(descriptor, instruction_content);

		const auto
		[
			instruction_name,
			instruction_content,
			trailing_expr,
			is_string_content
		] = util::parse_single_argument_command(instruction);

		if (instruction_name.empty())
		{
			// Try to process the raw instruction as an inline-update.
			if (process_inline_update_instruction(instruction_raw))
			{
				// Update the previous-instruction ID to reflect
				// the inline-update operation we just processed.
				this->prev_instruction_id = "update"_hs;

				// No further processing needed.
				return 1;
			}
			else
			{
				// Fallback to treating the 'headerless' instruction as a directive:
				auto directive_result = process_directive(content_source, content_index, thread_details, instruction);

				if (!directive_result.has_value() || (*directive_result > 0))
				{
					// No further processing needed.
					return directive_result;
				}
				else
				{
					// All other resolution methods have been exhausted,
					// log that we weren't able to handle the instruction.
					print_warn("Failed to process instruction: {}", instruction);

					return 1; // std::nullopt;
				}
			}
		}

		const auto instruction_id = hash(instruction_name).value();
		const auto prev_instruction_id = get_prev_instruction_id();

		// Handle continuous instructions:
		if (instruction_id != prev_instruction_id)
		{
			on_instruction_change(instruction_id, prev_instruction_id);
		}

		auto handle_instruction = [&](auto instruction_id, std::string_view instruction_name) -> EntityInstructionCount // std::opitonal<EntityInstructionCount>
		{
			auto error = [&instruction_id, &instruction_name, &instruction_content, content_index](std::string_view message)
			{
				throw std::runtime_error(util::format("[Instruction #{} ({}) @ Line #{}]: {} | \"{}\"", instruction_id, instruction_name, content_index, message, instruction_content));
			};

			switch (instruction_id)
			{
				// NOTE: `name` directive only works for unnamed threads.
				case "name"_hs:
				{
					if (instruction_content.empty())
					{
						print_error("Unable to process `name` directive: No name specified.");
					}
					else if (!set_thread_name(instruction_content, !is_string_content))
					{
						print_warn("Ignored `name` directive: Thread is already named.");
					}

					return 1;
				}

				case "update"_hs:
				{
					// Check if the input was comma-separated vs. standard assignment:
					if (instruction_content.contains(','))
					{
						process_update_instruction_from_csv(instruction_content);
					}
					else
					{
						process_inline_update_instruction(instruction_content);
					}

					return 1;
				}

				// Similar to a standard `when` block, but allows for multiple instances corresponding to each condition-block/event-type.
				case "when_each"_hs:
				{
					EntityInstructionCount instructions_processed = 0;

					// Process this sub-thread on each condition-block.
					process_trigger_expression
					(
						// TODO: Optimize. (Temporary string generated due to limitation of `std::regex`)
						std::string(instruction_content),

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
						opt_parsing_context
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
					auto condition = process_unified_condition_block(instruction_content, opt_parsing_context);

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
					auto condition = process_unified_condition_block(instruction_content, opt_parsing_context);

					if (condition)
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
					auto condition = process_unified_condition_block(instruction_content, opt_parsing_context);

					if (condition)
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

				// NOTE: We don't currently handle the `check_linked` field for `Pause`.
				case "pause"_hs:
				{
					//const auto check_linked = util::from_string<bool>(instruction_content).value_or(true);

					//return instruct_thread<Pause>(thread_details, check_linked);
					return instruct_thread<Pause>(util::optional_or(parse_thread_details(instruction_content), thread_details));
				}

				case "wake"_hs:
				case "resume"_hs:
				{
					return instruct_thread<Resume>(util::optional_or(parse_thread_details(instruction_content), thread_details));
				}

				case "link"_hs:
					return instruct<Link>();

				// NOTE: Unlike `link`, the `unlink` instruction supports both regular and alternative thread-designation.
				// 
				// This is due to the default assumption that threads are linked. As a consequence,
				// re-linking a thread requires referencing local thread instances directly.
				case "unlink"_hs:
					return instruct_thread<Unlink>(util::optional_or(parse_thread_details(instruction_content), thread_details));

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
					return instruct_thread<Detach>(util::optional_or(parse_thread_details(instruction_content), thread_details));

				// NOTE: For `start`, we currently allow both boolean input (restart behavior)
				// as well as the alternate thread-designation syntax.
				case "start"_hs:
				{
					if (const auto restart_existing = util::from_string<bool>(instruction_content))
					{
						return instruct_thread<Start>(thread_details, *restart_existing);
					}

					return instruct_thread<Start>
					(
						util::optional_or(parse_thread_details(instruction_content), thread_details),
						true
					);
				}

				case "restart"_hs:
				{
					return instruct_thread<Restart>(util::optional_or(parse_thread_details(instruction_content), thread_details));
				}

				// NOTE: We don't currently handle the `check_linked` field for `Stop`.
				case "terminate"_hs:
				case "stop"_hs:
				{
					//const auto check_linked = util::from_string<bool>(instruction_content).value_or(true);

					//return instruct_thread<Stop>(thread_details, check_linked);
					return instruct_thread<Stop>(util::optional_or(parse_thread_details(instruction_content), thread_details));
				}

				case "sleep"_hs:
				case "wait"_hs:
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
						else if (auto condition = process_unified_condition_block(instruction_content, opt_parsing_context))
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
					return from_file(instruction_content);

				case "embed"_hs:
					return from_lines(instruction_content);

				case "import"_hs:
				{
					auto imported_thread = sub_thread();

					imported_thread.from_file(instruction_content);

					return launch(imported_thread);
				}

				default:
				{
					// If this instruction ID couldn't be identified, assume it's a command instruction.
					return process_command_instruction
					(
						instruction_id, instruction_name,
						instruction_content,
						
						!is_string_content,

						(
							(is_string_content)
							? std::string_view {}      // No separation.
							: std::string_view { "," } // Comma separation.
						)
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

		print_warn("Unknown instruction detected: {} [#{}] | \"{}\"", instruction_name, instruction_id, instruction);

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
		EntityInstructionCount content_index = 0;
		EntityInstructionCount processed_instruction_count = 0;
		EntityInstructionCount active_skip_count = skip;

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

				processed_instruction_count += *result;

				content_index++;

				return true;
			}
		);

		return processed_instruction_count;
	}

	EntityInstructionCount EntityThreadBuilder::from_file(const std::filesystem::path& script_path, std::string_view separator, EntityInstructionCount skip)
	{
		std::filesystem::path resolved_path;

		if (opt_factory_context)
		{
			resolved_path = opt_factory_context->resolve_path
			(
				script_path,

				(
					(opt_base_path)
					? *opt_base_path
					: std::filesystem::path {}
				)
			);
		}

		const auto script_data = util::io::load_string
		(
			(resolved_path.empty())
			? script_path.string()
			: resolved_path.string()
		);

		if (!thread_has_name())
		{
			auto filename = resolved_path.filename(); filename.replace_extension();
			auto filename_str = filename.string();

			set_thread_name(filename_str);
		}

		return from_lines(script_data, separator, skip); // process(std::string_view(script_data), skip);
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
		return from_lines(lines, "\n", skip);
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

		StringHash directive_id
	)
	{
		using namespace entt::literals;

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

			directive_id
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

		StringHash directive_id
	)
	{
		using namespace entt::literals;

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

			directive_id
		);
	}

	// EntityThreadMultiBuilder:
	std::optional<EntityInstructionCount> EntityThreadMultiBuilder::process_directive_impl
	(
		const ContentSource& content_source,
		EntityInstructionCount content_index,

		const std::optional<EntityThreadInstruction>& thread_details,

		StringHash directive_id
	)
	{
		using namespace entt::literals;

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

			directive_id
		);
	}
}