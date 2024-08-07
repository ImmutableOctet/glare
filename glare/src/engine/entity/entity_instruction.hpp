#pragma once

#include "entity_state_action.hpp"
#include "event_trigger_condition.hpp"
#include "entity_descriptor_shared.hpp"
#include "entity_thread_cadence.hpp"

#include <engine/timer.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/indirect_meta_any.hpp>
#include <engine/meta/meta_variable_target.hpp>

#include <util/small_vector.hpp>
#include <util/variant.hpp>

#include <utility>
#include <optional>
#include <string>

namespace engine
{
	struct MetaTypeDescriptor;

	// Base type used for instructions that can affect other threads and entities.
	struct EntityThreadInstruction
	{
		// Defaults to this entity.
		EntityTarget target_entity = {};

		// If left default (0), this refers to the current thread.
		EntityThreadID thread_id = {};

		bool operator==(const EntityThreadInstruction&) const noexcept = default;
		bool operator!=(const EntityThreadInstruction&) const noexcept = default;
	};

	struct EntityInstructionDescriptor
	{
		//using RemoteInstruction = EntityDescriptorShared<MetaTypeDescriptor>;
		using RemoteInstruction = IndirectMetaAny;

		EntityInstructionDescriptor(EntityInstructionDescriptor&&) noexcept = default;
		EntityInstructionDescriptor(const EntityInstructionDescriptor&) = default;

		EntityInstructionDescriptor(IndirectMetaAny&& instruction) noexcept
			: instruction(std::move(instruction)) {}

		EntityInstructionDescriptor& operator=(const EntityInstructionDescriptor&) = default;
		EntityInstructionDescriptor& operator=(EntityInstructionDescriptor&&) noexcept = default;

		bool operator==(const EntityInstructionDescriptor&) const noexcept = default;
		bool operator!=(const EntityInstructionDescriptor&) const noexcept = default;

		RemoteInstruction instruction;
	};

	namespace instructions
	{
		using Thread = EntityThreadInstruction;
		using InstructionDescriptor = EntityInstructionDescriptor;

		using NoOp = std::monostate;

		struct ControlBlock
		{
			// Number of instructions included in this control-block.
			EntityInstructionCount size = {};

			bool operator==(const ControlBlock&) const noexcept = default;
			bool operator!=(const ControlBlock&) const noexcept = default;
		};

		struct LocalConditionControlBlock
		{
			// NOTE: Since this is a local condition-based control block,
			// the `condition` field is checked immediately.
			// Meaning the types used are expected to be components of a targeted entity.
			// 
			// This is in contrast to a `Yield` instruction, which halts
			// the thread until one of the event-types is triggered.
			EntityDescriptorShared<EventTriggerCondition> condition;

			// Bounds of this control-block.
			ControlBlock execution_range;

			bool operator==(const LocalConditionControlBlock&) const noexcept = default;
			bool operator!=(const LocalConditionControlBlock&) const noexcept = default;
		};

		struct Start : Thread
		{
			bool restart_existing = false;

			bool operator==(const Start&) const noexcept = default;
			bool operator!=(const Start&) const noexcept = default;
		};

		struct Stop : Thread
		{
			bool check_linked = true;

			bool operator==(const Stop&) const noexcept = default;
			bool operator!=(const Stop&) const noexcept = default;
		};

		struct Restart : Thread
		{
			bool operator==(const Restart&) const noexcept = default;
			bool operator!=(const Restart&) const noexcept = default;
		};

		struct Pause : Thread
		{
			bool check_linked = true;

			bool operator==(const Pause&) const noexcept = default;
			bool operator!=(const Pause&) const noexcept = default;
		};

		struct Resume : Thread
		{
			bool check_linked = true;

			bool operator==(const Resume&) const noexcept = default;
			bool operator!=(const Resume&) const noexcept = default;
		};

		struct Link
		{
			inline bool operator==(const Link&) const noexcept { return true;  } // = default;
			inline bool operator!=(const Link&) const noexcept { return false; } // = default;
		};

		struct Unlink : Thread
		{
			bool operator==(const Unlink&) const noexcept = default;
			bool operator!=(const Unlink&) const noexcept = default;
		};

		struct Attach : Thread
		{
			std::optional<EntityStateID> state_id = std::nullopt;

			bool check_linked = true;

			bool operator==(const Attach&) const noexcept = default;
			bool operator!=(const Attach&) const noexcept = default;
		};

		struct Detach : Thread
		{
			bool check_linked = true;

			bool operator==(const Detach&) const noexcept = default;
			bool operator!=(const Detach&) const noexcept = default;
		};

		struct Sleep : Thread
		{
			Timer::Duration duration = {};

			bool check_linked = true;

			bool operator==(const Sleep&) const noexcept = default;
			bool operator!=(const Sleep&) const noexcept = default;
		};

		struct Yield : Thread
		{
			EntityDescriptorShared<EventTriggerCondition> condition;

			bool operator==(const Yield&) const noexcept = default;
			bool operator!=(const Yield&) const noexcept = default;
		};

		//struct Step : Thread { ... };

		// TODO: Determine if remote-threads should be allowed to skip instructions.
		struct Skip : Thread
		{
			// Number of subsequent instructions to be skipped over.
			ControlBlock instructions_skipped; // EntityInstructionCount

			bool check_linked = true;

			bool operator==(const Skip&) const noexcept = default;
			bool operator!=(const Skip&) const noexcept = default;
		};

		//struct Revert  : Thread {};

		// TODO: Determine if remote-threads should be allowed to rewind other threads.
		struct Rewind : Thread
		{
			// Number of instructions to be moved backward by.
			EntityInstructionCount instructions_rewound = {}; // ControlBlock

			bool check_linked = true;

			bool operator==(const Rewind&) const noexcept = default;
			bool operator!=(const Rewind&) const noexcept = default;
		};

		struct MultiControlBlock
		{
			// Number of subsequent instructions to be
			// executed immediately upon reaching this one.
			ControlBlock included_instructions;

			bool operator==(const MultiControlBlock&) const noexcept = default;
			bool operator!=(const MultiControlBlock&) const noexcept = default;
		};

		struct CadenceControlBlock
		{
			EntityThreadCadence cadence = EntityThreadCadence::Default;
			//EntityThreadCadence prev_cadence = EntityThreadCadence::Default;

			// Number of subsequent instructions to be
			// included in this 'cadence' block.
			ControlBlock included_instructions = {};

			bool operator==(const CadenceControlBlock&) const noexcept = default;
			bool operator!=(const CadenceControlBlock&) const noexcept = default;
		};

		using ChangeCadence = CadenceControlBlock;

		struct IfControlBlock : LocalConditionControlBlock
		{
			bool operator==(const IfControlBlock&) const noexcept = default;
			bool operator!=(const IfControlBlock&) const noexcept = default;
		};

		struct FunctionCall
		{
			IndirectMetaAny function;

			bool operator==(const FunctionCall&) const noexcept = default;
			bool operator!=(const FunctionCall&) const noexcept = default;
		};

		struct CoroutineCall
		{
			IndirectMetaAny coroutine_function;

			bool operator==(const CoroutineCall&) const noexcept = default;
			bool operator!=(const CoroutineCall&) const noexcept = default;
		};

		struct AdvancedMetaExpression
		{
			IndirectMetaAny expr;

			bool operator==(const AdvancedMetaExpression&) const noexcept = default;
			bool operator!=(const AdvancedMetaExpression&) const noexcept = default;
		};

		struct VariableDeclaration
		{
			MetaVariableTarget variable_details;

			bool operator==(const VariableDeclaration&) const noexcept = default;
			bool operator!=(const VariableDeclaration&) const noexcept = default;
		};

		struct VariableAssignment : Thread
		{
			IndirectMetaAny assignment;
			std::optional<MetaVariableTarget> variable_details = std::nullopt;

			bool ignore_if_already_assigned = false;
			bool ignore_if_not_declared = false;

			bool operator==(const VariableAssignment&) const noexcept = default;
			bool operator!=(const VariableAssignment&) const noexcept = default;
		};

		struct EventCapture
		{
			MetaVariableTarget variable_details;
			MetaTypeID intended_type = {};

			bool operator==(const EventCapture&) const noexcept = default;
			bool operator!=(const EventCapture&) const noexcept = default;
		};

		struct Assert
		{
			EntityDescriptorShared<EventTriggerCondition> condition; // IndirectMetaAny

			std::optional<EntityDescriptorShared<std::string>> debug_message;
			std::optional<EntityDescriptorShared<std::string>> condition_representation;

			bool operator==(const Assert&) const noexcept = default;
			bool operator!=(const Assert&) const noexcept = default;
		};

		using Instruction = std::variant
		<
			NoOp,
				
			EntityStateAction,
			EntityStateTransitionAction,
			EntityStateCommandAction,
			EntityStateUpdateAction,
			EntityThreadSpawnAction,
			EntityThreadStopAction,
			EntityThreadPauseAction,
			EntityThreadResumeAction,
			EntityThreadAttachAction,
			EntityThreadDetachAction,
			EntityThreadUnlinkAction,
			EntityThreadSkipAction,
			EntityThreadRewindAction,

			Start,
			Restart,
			Stop,
			Pause,
			Resume,
			Link,
			Unlink,
			Attach,
			Detach,
			Sleep,
			Yield,

			//Step,

			Skip,
			Rewind,

			MultiControlBlock,
			CadenceControlBlock,
			IfControlBlock,

			FunctionCall,
			CoroutineCall,
			AdvancedMetaExpression,
			VariableDeclaration,
			VariableAssignment,
			EventCapture,
			Assert,

			InstructionDescriptor
		>;
	}

	struct EntityInstruction
	{
		using InstructionType = instructions::Instruction;

		inline EntityInstruction()
			: value(instructions::NoOp {}) {}

		inline EntityInstruction(InstructionType&& value)
			: value(std::move(value))
		{}

		inline EntityInstruction(MetaAny opaque_value)
			: value(instructions::NoOp {})
		{
			if (!opaque_value)
			{
				return;
			}

			util::for_each_variant_type<InstructionType>
			(
				[this, &opaque_value]<typename T>()
				{
					if (static_cast<bool>(*this))
					{
						return;
					}

					if (auto exact_type = opaque_value.try_cast<T>())
					{
						this->value = std::move(*exact_type); // *exact_type;
					}
				}
			);
		}

		template <typename T, typename=std::enable_if_t<util::variant_contains_v<InstructionType, T>, T>>
		EntityInstruction(T&& value)
			: value(std::move(value))
		{}

		template <typename T>
		static EntityInstruction from_type(T value)
		{
			return EntityInstruction(InstructionType(std::move(value)));
		}

		inline static EntityInstruction from_meta_any(MetaAny opaque_value)
		{
			return EntityInstruction(std::move(opaque_value));
		}

		EntityInstruction(const EntityInstruction&) = default;
		EntityInstruction(EntityInstruction&&) noexcept = default;

		EntityInstruction& operator=(const EntityInstruction&) = default;
		EntityInstruction& operator=(EntityInstruction&&) noexcept = default;

		bool operator==(const EntityInstruction&) const noexcept = default;
		bool operator!=(const EntityInstruction&) const noexcept = default;

		std::size_t type_index() const
		{
			return value.index();
		}

		operator const InstructionType&() const
		{
			return value;
		}

		explicit operator bool() const
		{
			return (type_index() != util::variant_index<InstructionType, instructions::NoOp>());
		}

		InstructionType value;
	};

	using EntityInstructionType = EntityInstruction::InstructionType;
}