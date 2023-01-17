#pragma once

#include "types.hpp"
#include "serial.hpp"

#include "entity_state_action.hpp"
#include "entity_thread_description.hpp"
#include "entity_descriptor_shared.hpp"

#include <util/json.hpp>

#include <filesystem>
#include <string_view>
#include <optional>
#include <utility>
#include <variant>
#include <string>
#include <functional>

namespace engine
{
	class EntityDescriptor;

	struct CommandParsingContext;
	struct EntityThreadDescription;
	struct MetaTypeDescriptor;
	struct EntityFactoryContext;

	class EntityThreadBuilderContext
	{
		public:
			EntityThreadBuilderContext
			(
				EntityDescriptor& descriptor,
				EntityThreadDescription& thread,
				const EntityFactoryContext* opt_factory_context = nullptr,
				const std::filesystem::path* opt_base_path = nullptr,
				const CommandParsingContext* opt_command_context = nullptr
			);

			EntityThreadIndex thread_index() const;

			inline EntityThreadDescription& get_thread()
			{
				return thread;
			}

			inline const EntityThreadDescription& get_thread() const
			{
				return thread;
			}

			inline EntityDescriptor& get_descriptor()
			{
				return descriptor;
			}

			inline const EntityDescriptor& get_descriptor() const
			{
				return descriptor;
			}
		protected:
			EntityDescriptor& descriptor;
			EntityThreadDescription& thread;

			const EntityFactoryContext* opt_factory_context = nullptr;
			const std::filesystem::path* opt_base_path = nullptr;
			const CommandParsingContext* opt_command_context = nullptr;
	};

	// Handles processing steps for one or more `EntityThreadDescription` objects.
	class EntityThreadBuilder : public EntityThreadBuilderContext
	{
		public:
			using InstructionID = StringHash;
			using InstructionIndex = EntityInstructionIndex;
			using InstructionCount = EntityInstructionCount;

			using ContentSource = std::variant
			<
				std::reference_wrapper<const util::json>,
				std::string_view
			>;

			EntityThreadBuilder
			(
				const EntityThreadBuilderContext& context,
				std::string_view opt_thread_name = {}
			);

			EntityThreadBuilder
			(
				EntityDescriptor& descriptor,
				EntityThreadDescription& thread,
				std::string_view opt_thread_name={},
				const EntityFactoryContext* opt_factory_context=nullptr,
				const std::filesystem::path* opt_base_path=nullptr,
				const CommandParsingContext* opt_command_context=nullptr
			);

			EntityThreadBuilder
			(
				EntityDescriptor& descriptor,
				std::string_view opt_thread_name={},
				const EntityFactoryContext* opt_factory_context=nullptr,
				const std::filesystem::path* opt_base_path=nullptr,
				const CommandParsingContext* opt_command_context=nullptr
			);

			virtual ~EntityThreadBuilder();

			EntityInstructionCount process(const util::json& content, EntityInstructionCount skip=0);
			EntityInstructionCount process(std::string_view lines, EntityInstructionCount skip=0);
			EntityInstructionCount process(const ContentSource& content, EntityInstructionCount skip=0);

			// NOTE: The return-value of this function is a non-owning pointer to the affected descriptor.
			MetaTypeDescriptor* process_update_instruction_from_values
			(
				std::string_view type_name,
				std::string_view member_name,
				std::string_view value_raw,
				std::string_view entity_ref_expr
			);

			// NOTE: The return-value of this function is a non-owning pointer to the affected descriptor.
			MetaTypeDescriptor* process_update_instruction_from_csv(std::string_view instruction_separated, std::string_view separator=",");

			// NOTE: The return-value of this function is a non-owning pointer to the affected descriptor.
			MetaTypeDescriptor* process_inline_update_instruction(std::string_view instruction);

			// Attempts to process one or more instructions.
			// 
			// The return value of this function indicates how many
			// instructions from `content_source` were processed.
			// 
			// If this function returns `std::nullopt`, the active scope
			// this thread-builder was working on should be assumed to have ended.
			// (This is usually the result of an `end` directive)
			// 
			// The `content_source` and `content_index` arguments
			// indicate the location at which `instruction` was found.
			//
			// See also: `process`
			std::optional<EntityInstructionCount> process_instruction
			(
				const ContentSource& content_source,
				EntityInstructionCount content_index,
				std::string_view instruction_raw
			);

			// Attempts to emit a command instruction from the input given.
			EntityInstructionCount process_command_instruction
			(
				InstructionID instruction_id,
				std::string_view instruction_name,
				std::string_view instruction_content,
				bool resolve_values,
				std::string_view separator=","
			);

			// Attempts to handle the input provided as a standalone directive.
			// (i.e. declaration taking no parameters)
			std::optional<EntityInstructionCount> process_directive
			(
				const ContentSource& content_source,
				EntityInstructionCount content_index,

				const std::optional<EntityThreadInstruction>& thread_details,

				std::string_view directive
			);

			// Attempts to assign the name of the targeted `thread`.
			// 
			// NOTE: If a name has already been assigned, the
			// `force` argument must be true to ensure assignment.
			std::optional<EntityThreadID> set_thread_name(std::string_view thread_name, bool resolve_name=false, bool force=false);

			// Attempts to retrieve the name (thread ID) associated with the targeted `thread`.
			std::optional<EntityThreadID> get_thread_name() const;

			// Identifies if the target `thread` currently has a name associated.
			bool thread_has_name() const;

			// Retrieves the index at which new instructions may be emitted.
			InstructionIndex get_instruction_index();

			// Shorthand for `process`. Allows for `<<` operator chaining.
			inline EntityThreadBuilder& operator<<(const auto& content)
			{
				process(content);

				return *this;
			}

			// Retrieves an ID associated with the last instruction processed.
			inline InstructionID get_prev_instruction_id() const
			{
				return prev_instruction_id;
			}

			// Emits an instruction for the target `thread`.
			template <typename InstructionType, typename ...Args>
			inline InstructionType& emit(Args&&... args)
			{
				auto& stored_instance = thread.instructions.emplace_back
				(
					EntityInstruction::InstructionType
					{
						InstructionType
						{
							std::forward<Args>(args)...
						}
					}
				);

				auto& opaque_instruction = stored_instance.value;

				auto& resolved_instruction = std::get<InstructionType>(opaque_instruction);

				return resolved_instruction;
			}

			// Emits an instruction for the target `thread`.
			template <typename InstructionType>
			inline InstructionType& emit(InstructionType&& processed_instruction)
			{
				return emit<InstructionType, InstructionType&&>(std::move(processed_instruction));
			}

			// Shorthand for `emit` + `return 1`.
			template <typename InstructionType, typename ...Args>
			inline EntityInstructionCount instruct(Args&&... args)
			{
				emit<InstructionType>(std::forward<Args>(args)...);

				return 1;
			}

			// Shorthand for `emit` + `return 1`.
			template <typename InstructionType>
			inline EntityInstructionCount instruct(InstructionType&& processed_instruction)
			{
				emit(std::forward<InstructionType>(processed_instruction));

				return 1;
			}

			// Shorthand for `instruct`/`emit` for thread control-flow instructions.
			template <typename InstructionType, typename ...Args>
			inline EntityInstructionCount instruct_thread(const EntityThreadInstruction& thread_instruction, Args&&... args)
			{
				return instruct<InstructionType>
				(
					thread_instruction.target_entity,
					thread_instruction.thread_id,

					std::forward<Args>(args)...
				);
			}

			// Shorthand for `instruct`/`emit` control-flow instructions.
			template <typename InstructionType, typename ...Args>
			inline EntityInstructionCount instruct_thread(const std::optional<EntityThreadInstruction>& opt_thread_instruction, Args&&... args)
			{
				if (opt_thread_instruction)
				{
					return instruct<InstructionType>
					(
						opt_thread_instruction->target_entity, // std::move(...)
						opt_thread_instruction->thread_id,     // std::move(...)

						std::forward<Args>(args)...
					);
				}

				return instruct<InstructionType>
				(
					// Self target.
					EntityTarget {},

					// This thread. (No thread ID)
					std::optional<EntityThreadID>(std::nullopt),

					std::forward<Args>(args)...
				);
			}

			// Shorthand for `instruct`/`emit` control-flow instructions.
			template <typename InstructionType>
			inline EntityInstructionCount instruct_thread(InstructionType&& processed_instruction)
			{
				return instruct(std::forward<InstructionType>(processed_instruction));
			}

			// Emits an instruction to launch `thread_index`.
			EntityInstructionCount launch(EntityThreadIndex thread_index);

			// Emits an instruction to launch the thread referenced by `context`.
			EntityInstructionCount launch(const EntityThreadBuilderContext& context);

			// Returns a reference to the context this builder was created with.
			const EntityThreadBuilderContext& context() const;

			// Generates a sub-context for a given `target_sub_thread`.
			EntityThreadBuilderContext sub_context(EntityThreadDescription& target_sub_thread) const;

			// Generates a sub-context and allocates a new thread to be associated.
			EntityThreadBuilderContext sub_context() const;

			// Generates a new thread-builder using this builder's targeted `descriptor`.
			EntityThreadBuilder sub_thread(std::string_view thread_name={});

			// Loads instruction content from a file located at `script_path`.
			// NOTE: This uses the `opt_base_path` field to better resolve the path specified.
			EntityInstructionCount from_file(const std::filesystem::path& script_path, std::string_view separator="\n", EntityInstructionCount skip=0);
		protected:
			// Retrieves a mutable reference to the current multi-line update instruction.
			// 
			// If a multi-line update instruction has not been established, this will
			// construct a new instance that is owned by this thread-builder.
			EntityStateUpdateAction& get_update_instruction(EntityTarget target={});

			// Flushes the current multi-line update instruction to `thread`.
			bool flush_update_instruction();

			// Handles processing of a standalone entity-update block. (JSON `object`)
			bool on_update_instruction(const util::json& update_entry);

			// Triggered any time the active instruction ID changes to something different. (Executes regularly)
			// This is useful for implementing things like compound update-instructions defined by multiple lines.
			virtual bool on_instruction_change(InstructionID instruction_id, InstructionID prev_instruction_id);

			// NOTE: We add the `scope_offset` to the result of `callback` to account
			// for both the initial block-start instruction as well as the block-end instruction.
			template <typename ControlBlockType, typename Callback>
			inline EntityInstructionCount control_block(ControlBlockType& block_out, Callback&& callback, EntityInstructionCount scope_offset=2)
			{
				// Retrieve the instruction-index prior to executing `callback`.
				const auto instruction_index = get_instruction_index();

				// Store the result of `callback`.
				// (Entries/lines processed; different from instructions emitted)
				const auto result = callback();

				// Retrieve the instruction-index after executing `callback`.
				const auto updated_instruction_index = get_instruction_index();

				// Determine the number of instructions emitted by `callback`.
				// (Difference between the two snapshots)
				const auto block_size =
				(
					static_cast<EntityInstructionCount>(updated_instruction_index)
					-
					static_cast<EntityInstructionCount>(instruction_index)
				);

				// Update the size of the specified control-block.
				block_out.size = block_size;

				// Return the result of `callback` (i.e. Number of 'lines' processed)
				// + a `scope_offset`. (Meant to represent the beginning and end instructions of the scope)
				return (result + scope_offset);
			}

			EntityInstructionCount from_array(const util::json& thread_content, EntityInstructionCount skip=0);
			EntityInstructionCount from_object(const util::json& thread_content, bool flush=true);
			EntityInstructionCount from_lines(std::string_view lines, std::string_view separator="\n", EntityInstructionCount skip=0);

			EntityInstructionCount generate_if_block
			(
				EventTriggerCondition&& condition,

				const ContentSource& content_source,
				EntityInstructionCount content_index
			);

			EntityInstructionCount generate_while_block
			(
				EventTriggerCondition&& condition,

				const ContentSource& content_source,
				EntityInstructionCount content_index
			);

			EntityInstructionCount generate_when_block
			(
				EventTriggerCondition&& condition,
				
				const ContentSource& content_source,
				EntityInstructionCount content_index
			);

			EntityInstructionCount generate_multi_block
			(
				const ContentSource& content_source,
				EntityInstructionCount content_index
			);

			virtual std::optional<EntityInstructionCount> process_directive_impl
			(
				const ContentSource& content_source,
				EntityInstructionCount content_index,

				const std::optional<EntityThreadInstruction>& thread_details,

				StringHash directive_id
			);

			InstructionID prev_instruction_id = 0;
			std::optional<EntityStateUpdateAction> current_update_instruction = std::nullopt;
	};

	// Builds a `when` block.
	//using EntityThreadWhenBuilder = EntityThreadBuilder;

	// Builds an `if` block.
	class EntityThreadIfBuilder final : public EntityThreadBuilder
	{
		public:
			EntityThreadIfBuilder
			(
				const EntityThreadBuilderContext& context
			);

			inline bool has_else() const
			{
				return exited_on_else;
			}

		protected:
			std::optional<EntityInstructionCount> process_directive_impl
			(
				const ContentSource& content_source,
				EntityInstructionCount content_index,

				const std::optional<EntityThreadInstruction>& thread_details,

				StringHash directive
			) override;

			bool exited_on_else = false;
	};

	// Builds an `while` block.
	class EntityThreadWhileBuilder final : public EntityThreadBuilder
	{
		public:
			EntityThreadWhileBuilder
			(
				const EntityThreadBuilderContext& context
			);
		protected:
			std::optional<EntityInstructionCount> process_directive_impl
			(
				const ContentSource& content_source,
				EntityInstructionCount content_index,

				const std::optional<EntityThreadInstruction>& thread_details,

				StringHash directive
			) override;
	};

	// Builds a `multi` block.
	class EntityThreadMultiBuilder final : public EntityThreadBuilder
	{
		public:
			using EntityThreadBuilder::EntityThreadBuilder;

		protected:
			std::optional<EntityInstructionCount> process_directive_impl
			(
				const ContentSource& content_source,
				EntityInstructionCount content_index,

				const std::optional<EntityThreadInstruction>& thread_details,

				StringHash directive
			) override;
	};
}