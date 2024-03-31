#pragma once

#include "types.hpp"
#include "event_trigger_condition.hpp"
#include "entity_variables.hpp"
#include "entity_thread_cadence.hpp"

#include "entity_thread_fiber.hpp"

#include <optional>
#include <memory>

namespace engine
{
	struct EntityThreadFlags
	{
		// If a thread is detached, it will continue to execute even when the active state changes.
		// (By default, threads are terminated upon state change; i.e. attached)
		//
		// NOTES:
		// 
		// * A thread can be detached (meaning it'll run indefinitely), but still
		//   be linked (meaning it can be referenced by name). This allows the user to control
		//   when a thread stops execution, rather than being subjected to forced termination.
		//
		// * All threads (regardless of attachment or link status) are terminated when
		//   the thread instance is destroyed or otherwise removed. (see: `EntityThreadComponent`)
		//   
		//   By definition, this means that a thread cannot outlive the entity that spawned it.
		bool is_detached : 1 = false;

		// By default, threads take on a 'linked' state, which ensures that
		// only one instance of a given thread is executing per-entity.
		//
		// If a thread is unlinked, it will continue to execute until stopped/terminated, but will
		// not block the creation of another instance pointing to the same `EntityThreadDescription` object.
		//
		// Linked threads are able to be referenced by name.
		bool is_linked : 1 = true;

		// Indicates if this thread has been paused.
		// 
		// A paused thread must be resumed by another thread (or rule-trigger) to proceed.
		bool is_paused : 1 = false;

		// Indicates if this thread is currently awaiting an event.
		// 
		// A thread that is yielding cannot continue execution
		// until its trigger condition has been satisfied.
		// 
		// See also: `EntityListener`
		bool is_yielding : 1 = false;

		// Indicates whether this thread has reached a termination point.
		bool is_complete : 1 = false;

		// The cadence of this thread; used to control rate of execution.
		EntityThreadCadence cadence = EntityThreadCadence::Default;

		// A thread is considered suspended if it cannot continue
		// work until an external operation takes place.
		// (e.g. a 'resume' operation or a yield-condition being met)
		bool is_suspended() const
		{
			return (is_paused || is_yielding || is_complete);
		}

		bool is_sleeping() const
		{
			return is_paused;
		}
	};

	struct EntityThread : public EntityThreadFlags // protected
	{
		public:
			using InstructionIndex = EntityInstructionIndex;

			using ThreadLocalVariables = EntityVariables<8>;

			EntityThread
			(
				const EntityThreadFlags& flags,

				EntityThreadIndex thread_index,
				InstructionIndex first_instruction=0,
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

			bool sleep()
			{
				return pause();
			}

			bool wake()
			{
				return resume();
			}

			bool play()
			{
				return resume();
			}

			void clear_fiber()
			{
				active_fiber = {};
			}

			void set_fiber(EntityThreadFiber&& fiber)
			{
				active_fiber = std::move(fiber);
			}

			EntityThreadFiber& get_fiber()
			{
				return active_fiber;
			}

			bool has_fiber() const
			{
				return active_fiber.exists();
			}

			// Attempts to allocate a `ThreadLocalVariables` object, managed internally.
			// If a `ThreadLocalVariables` object has already been allocated, this will return a pointer to the existing instance.
			// The value returned is a non-owning pointer to the remote object. (see `variables`)
			ThreadLocalVariables* get_variables();

			EntityInstructionCount skip(EntityInstructionCount forward_stride);
			EntityInstructionCount rewind(EntityInstructionCount backward_stride);

			// An identifier used to locate the underlying
			// `EntityThreadDescription` data for this thread.
			EntityThreadIndex thread_index;

			// An index pointing to the next instruction,
			// found in `EntityThreadDescription`.
			InstructionIndex next_instruction = 0;

			// An index representing the state this thread was instantiated from.
			std::optional<EntityStateIndex> state_index;

			// The active fiber to be executed, if any.
			EntityThreadFiber active_fiber;

			// Optional pointer to a container of thread-local variables.
			std::shared_ptr<ThreadLocalVariables> variables;
	};
}