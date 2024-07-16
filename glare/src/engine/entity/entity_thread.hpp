#pragma once

#include "types.hpp"
#include "event_trigger_condition.hpp"
#include "entity_variables.hpp"
#include "entity_thread_flags.hpp"
#include "entity_thread_cadence.hpp"
#include "entity_thread_fiber.hpp"

#include <optional>
#include <memory>

namespace engine
{
	struct EntityThread : public EntityThreadFlags // protected
	{
		public:
			using InstructionIndex = EntityInstructionIndex;

			using ThreadLocalVariables = EntityVariables<8>;

			EntityThread() = default;

			EntityThread
			(
				const EntityThreadFlags& flags,

				EntityThreadIndex thread_index,
				EntityThreadID thread_name={},
				EntityThreadID parent_thread_name={},
				std::optional<EntityStateIndex> state_index=std::nullopt,
				InstructionIndex first_instruction={}
			);

			EntityThread
			(
				const EntityThreadFlags& flags,
				EntityThreadFiber&& fiber,
				EntityThreadID thread_name={},
				EntityThreadID parent_thread_name={},
				std::optional<EntityStateIndex> state_index=std::nullopt
			);

			EntityThread
			(
				const EntityThreadFlags& flags,
				ScriptFiber&& fiber,
				EntityThreadID thread_name={},
				EntityThreadID parent_thread_name={},
				std::optional<EntityStateIndex> state_index=std::nullopt
			);

			EntityThread(EntityThread&&) noexcept = default;
			EntityThread(const EntityThread&) = delete;

			EntityThread& operator=(EntityThread&&) noexcept = default;
			EntityThread& operator=(const EntityThread&) = delete;

			bool pause();
			bool resume();
			bool link();
			bool unlink();
			bool attach(std::optional<EntityStateIndex> state_index=std::nullopt, bool keep_existing_state=false);
			bool detach();
			bool yield();
			bool unyield(EntityInstructionCount instruction_advancement=1);

			inline bool sleep()
			{
				return pause();
			}

			inline bool wake()
			{
				return resume();
			}

			inline bool play()
			{
				return resume();
			}

			inline void clear_fiber()
			{
				active_fiber = {};
			}

			inline void set_fiber(EntityThreadFiber&& fiber)
			{
				active_fiber = std::move(fiber);
			}

			inline void set_fiber(ScriptFiber&& fiber)
			{
				active_fiber = { std::move(fiber) };
			}

			inline EntityThreadFiber& get_fiber()
			{
				return active_fiber;
			}

			inline const EntityThreadFiber& get_fiber() const
			{
				return active_fiber;
			}

			inline bool has_fiber() const
			{
				return active_fiber.exists();
			}

			inline std::optional<EntityStateIndex> get_state_index() const
			{
				if (has_state_index())
				{
					return state_index;
				}

				return std::nullopt;
			}

			inline bool has_state_index() const
			{
				return (state_index != ENTITY_STATE_INDEX_INVALID);
			}

			// Attempts to allocate a `ThreadLocalVariables` object, managed internally.
			// If a `ThreadLocalVariables` object has already been allocated, this will return a pointer to the existing instance.
			// The value returned is a non-owning pointer to the remote object. (see `variables`)
			ThreadLocalVariables* get_variables();

			EntityInstructionCount skip(EntityInstructionCount forward_stride);
			EntityInstructionCount rewind(EntityInstructionCount backward_stride);

			// An identifier used to locate the underlying
			// `EntityThreadDescription` data for this thread.
			EntityThreadIndex thread_index = ENTITY_THREAD_INDEX_INVALID;

			// An index pointing to the next instruction,
			// found in `EntityThreadDescription`.
			InstructionIndex next_instruction = ENTITY_INSTRUCTION_INDEX_INVALID;

			// An index representing the state this thread was instantiated from.
			EntityStateIndex state_index = ENTITY_STATE_INDEX_INVALID;

			// An identifier used to represent this thread.
			EntityThreadID thread_id = {};

			// An identifier used to represent this thread's parent.
			EntityThreadID parent_thread_id = {};

			// Optional pointer to a container of thread-local variables.
			std::shared_ptr<ThreadLocalVariables> variables;

			// The active fiber to be executed, if any.
			EntityThreadFiber active_fiber;

		private:
			static constexpr EntityStateIndex resolve_state_index(std::optional<EntityStateIndex> state_index)
			{
				if (state_index)
				{
					return *state_index;
				}

				return ENTITY_STATE_INDEX_INVALID;
			}
	};
}