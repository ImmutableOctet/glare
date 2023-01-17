#pragma once

#include "entity_state_action.hpp"
#include "event_trigger_condition.hpp"
#include "entity_descriptor_shared.hpp"

#include <engine/timer.hpp>

#include <util/small_vector.hpp>

namespace engine
{
	// Base type used for instructions that can affect other threads and entities.
	struct EntityThreadInstruction
	{
		// Defaults to this entity.
		EntityTarget target_entity = {};

		// If empty, this refers to the current thread.
		std::optional<EntityThreadID> thread_id = std::nullopt;
	};

	namespace instructions
	{
		using Thread = EntityThreadInstruction;

		using NoOp   = std::monostate;

		struct ControlBlock
		{
			// Number of instructions included in this control-block.
			EntityInstructionCount size;
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
		};

		struct Start   : Thread { bool restart_existing = false; };
		struct Stop    : Thread { bool check_linked = true; };
		struct Restart : Thread {};
		struct Pause   : Thread { bool check_linked = true; };
		struct Resume  : Thread { bool check_linked = true; };

		struct Link {};
		struct Unlink : Thread {};

		struct Attach : Thread
		{
			std::optional<EntityStateID> state_id = std::nullopt;

			bool check_linked = true;
		};

		struct Detach : Thread
		{
			bool check_linked = true;
		};

		struct Sleep   : Thread { Timer::Duration duration; bool check_linked = true; };
		struct Yield   : Thread { EntityDescriptorShared<EventTriggerCondition> condition; }; // { EventTriggerCondition condition; };

		//struct Step    : Thread {};

		// TODO: Determine if remote-threads should be allowed to skip instructions.
		struct Skip : Thread
		{
			// Number of subsequent instructions to be skipped over.
			ControlBlock instructions_skipped; // EntityInstructionCount

			bool check_linked = true;
		};

		//struct Revert  : Thread {};

		// TODO: Determine if remote-threads should be allowed to rewind other threads.
		struct Rewind : Thread
		{
			// Number of instructions to be moved backward by.
			EntityInstructionCount instructions_rewound; // ControlBlock

			bool check_linked = true;
		};

		struct MultiControlBlock
		{
			// Number of subsequent instructions to be
			// executed immediately upon reaching this one.
			ControlBlock included_instructions;
		};

		struct IfControlBlock : LocalConditionControlBlock {};

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
			IfControlBlock
		>;
	}

	struct EntityInstruction
	{
		public:
			using InstructionType = instructions::Instruction;

			InstructionType value;

			inline std::size_t type_index() const
			{
				return value.index();
			}

			inline operator const InstructionType&() const
			{
				return value;
			}
	};

	using EntityInstructionType = EntityInstruction::InstructionType;
}