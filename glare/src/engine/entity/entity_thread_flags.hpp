#pragma once

#include "entity_thread_cadence.hpp"

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
		inline bool is_suspended() const
		{
			return (is_paused || is_yielding || is_complete);
		}

		inline bool is_sleeping() const
		{
			return is_paused;
		}
	};
}