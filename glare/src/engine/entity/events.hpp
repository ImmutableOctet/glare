#pragma once

#include "types.hpp"

//#include <engine/meta/types.hpp>
//#include <engine/meta/meta_variable_target.hpp>
#include <engine/meta/meta_any.hpp>
#include <engine/meta/meta_variable_scope.hpp>

namespace engine
{
	// Triggers when a state change takes place.
	struct OnStateChange
	{
		Entity entity;

		EntityStateInfo from;
		EntityStateInfo to;

		// Indicates whether the state described by `to` has already been activated.
		// 
		// For most state-change scenarios, this value will be true, meaning the initial changes
		// imposed by switching to the new state have been applied. -- However, a state can utilize
		// a self-imposed delay in activation, which will delay changes for a certain amount of time.
		bool state_activated = true;
	};

	// Triggers after a state has finished activating.
	struct OnStateActivate
	{
		Entity entity;

		EntityStateInfo state;
	};

	struct ThreadEvent
	{
		using LocalThreadIndex = EntityThreadIndex; // std::size_t;

		// The entity that this thread is executing for.
		Entity entity;

		// An index to the thread-description that this thread is based on.
		EntityThreadIndex thread_index;

		// The name/identifier associated with this thread.
		EntityThreadID thread_id;

		// An index to this local thread-instance tied to `entity`.
		// 
		// After this event has taken place, there is no
		// guarantee that this index will remain valid.
		LocalThreadIndex local_instance;

		// An index pointing to the last instruction executed.
		EntityInstructionIndex last_instruction_index;
	};

	// Triggered when a thread is spawned formally.
	struct OnThreadSpawn : ThreadEvent {};

	// Triggered when a thread completes execution normally.
	struct OnThreadComplete : ThreadEvent {};

	// Triggered when a thread formally finishes execution via termination.
	struct OnThreadTerminated : ThreadEvent {};

	// Triggered when a thread formally pauses execution.
	struct OnThreadPaused : ThreadEvent {};

	// Triggered when a thread formally resumes execution.
	struct OnThreadResumed : ThreadEvent {};

	// Triggered when a thread is formally (re-)attached.
	struct OnThreadAttach : ThreadEvent {};

	// Triggered when a thread is formally detached.
	struct OnThreadDetach : ThreadEvent {};

	// Triggered when a thread is formally unlinked.
	struct OnThreadUnlink : ThreadEvent {};

	struct OnThreadVariableUpdate : ThreadEvent
	{
		MetaSymbolID resolved_variable_name;
		MetaVariableScope variable_scope;

		MetaAny variable_update_result;
	};

	struct OnThreadEventCaptured : ThreadEvent
	{
		MetaTypeID event_type_id;
	};

	// Triggered when one or more of an entity's threads have been updated.
	struct OnEntityThreadsUpdated
	{
		Entity entity;
	};

	using OnThreadStart = OnThreadSpawn;
}