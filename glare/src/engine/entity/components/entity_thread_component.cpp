#include "entity_thread_component.hpp"

#include <engine/entity/entity_thread.hpp>
#include <engine/entity/entity_thread_description.hpp>
#include <engine/entity/entity_descriptor.hpp>
#include <engine/entity/entity_state.hpp>

#include <engine/script/script.hpp>

#include <iterator>
#include <algorithm>

namespace engine
{
	EntityThreadComponent::ThreadGlobalVariables* EntityThreadComponent::get_global_variables() // const
	{
		if (!global_variables)
		{
			global_variables = std::make_shared<ThreadGlobalVariables>();
		}

		return global_variables.get();
	}

	// NOTE: The `thread_id` argument is currently unused,
	// since thread names/IDs normally correspond to `EntityDescriptor` entries.
	EntityThread* EntityThreadComponent::start_thread
	(
		ScriptFiber&& fiber,

		std::optional<EntityStateIndex> state_index,
		
		EntityThreadID thread_id,
		EntityThreadID parent_thread_id,

		const EntityThreadFlags& flags,

		ScriptHandle script_handle
	)
	{
		if (!fiber)
		{
			return {};
		}

		auto& thread_out = threads.emplace_back
		(
			flags,
			std::move(fiber),
			thread_id,
			parent_thread_id,
			state_index
		);

		if (script_handle)
		{
			auto& script = script_handle;

			thread_out.active_fiber.script = script;

			auto& thread_handle = thread_out;

			script->set_executing_thread(thread_handle);
		}

		return &thread_out;
	}

	EntityThread* EntityThreadComponent::start_thread
	(
		EntityThreadIndex thread_index,

		std::optional<EntityStateIndex> state_index,

		bool check_existing,
		bool check_linked,
		bool restart_existing,
		
		EntityThreadID thread_id,
		EntityThreadID parent_thread_id,

		const EntityThreadFlags& flags
	)
	{
		if (check_existing)
		{
			auto* thread = get_thread(thread_index, check_linked);

			if (thread)
			{
				if (restart_existing)
				{
					*thread =
					{
						flags,
						thread_index,
						thread_id,
						parent_thread_id,
						state_index
					};

					return thread;
				}

				// TODO: Look into resume/restart if the thread is still active.
				//return thread;
				return {};
			}
		}

		auto& thread_out = threads.emplace_back
		(
			flags,
			thread_index,
			thread_id,
			parent_thread_id,
			state_index
		);

		return &thread_out;
	}

	EntityThread* EntityThreadComponent::start_thread
	(
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id,

		std::optional<EntityStateIndex> state_index,
		bool check_existing,
		bool check_linked,
		bool restart_existing,

		EntityThreadID parent_thread_id,

		const EntityThreadFlags& flags
	)
	{
		return start_thread
		(
			descriptor.get_thread_index(thread_id).value_or(ENTITY_THREAD_INDEX_INVALID),
			state_index,
			check_existing, check_linked, restart_existing,
			thread_id, parent_thread_id,
			flags
		);
	}

	EntityThread* EntityThreadComponent::start_thread
	(
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id,

		std::optional<EntityStateIndex> state_index,
		
		bool check_existing,
		bool check_linked,
		bool restart_existing,

		EntityThreadID parent_thread_id
	)
	{
		const auto thread_description = descriptor.get_thread_by_id(thread_id);

		return start_thread
		(
			descriptor, thread_id, state_index,
			check_existing, check_linked, restart_existing,

			parent_thread_id,

			EntityThreadFlags
			{
				.cadence = (thread_description)
					? thread_description->cadence
					: EntityThreadCadence::Default
			}
		);
	}

	std::size_t EntityThreadComponent::start_threads
	(
		const EntityDescriptor& descriptor,
		const EntityThreadRange& thread_range,
		
		std::optional<EntityStateIndex> state_index,

		EntityThreadID parent_thread_id,

		bool restart_existing
	)
	{
		std::size_t started_thread_count = 0;

		iterate_thread_range
		(
			thread_range,
			[this, &descriptor, &state_index, parent_thread_id, restart_existing, &started_thread_count](EntityThreadIndex thread_index)
			{
				const auto& thread_description = descriptor.get_thread(thread_index);

				const auto start_result = start_thread
				(
					thread_index, state_index,
					true, true, restart_existing,

					thread_description.name(),
					parent_thread_id,

					EntityThreadFlags
					{
						.cadence = thread_description.cadence
					}
				);

				if (start_result)
				{
					started_thread_count++;
				}
			}
		);

		return started_thread_count;
	}

	std::size_t EntityThreadComponent::start_threads
	(
		const EntityDescriptor& descriptor,
		EntityStateIndex state_index,
		EntityThreadID parent_thread_id,
		bool restart_existing
	)
	{
		const auto* state = descriptor.get_state_by_index(state_index);

		if (!state)
		{
			return 0;
		}

		return start_threads(descriptor, *state, state_index, parent_thread_id, restart_existing);
	}

	std::size_t EntityThreadComponent::start_threads
	(
		const EntityDescriptor& descriptor,
		const EntityState& state,
		EntityStateIndex state_index,
		EntityThreadID parent_thread_id,
		bool restart_existing
	)
	{
		std::size_t threads_started = 0;

		for (const auto& threads : state.immediate_threads)
		{
			threads_started += start_threads(descriptor, threads, state_index, parent_thread_id, restart_existing);
		}

		return threads_started;
	}

	// The return value of this method indicates if the specified
	// thread was terminated (true) or detached (false).
	// 
	// If no thread could be found, true will be returned.
	bool EntityThreadComponent::stop_thread
	(
		EntityThreadIndex thread_index,
		bool check_linked
	)
	{
		auto* thread = get_thread(thread_index, check_linked);

		if (!thread)
		{
			// Notify the user that the thread is already terminated.
			// (Or wouldn't otherwise conflict with creating a new one)
			return true;
		}

		return static_cast<bool>(stop_thread_impl(*thread));
	}

	bool EntityThreadComponent::stop_thread_by_id(EntityThreadID thread_id, bool check_linked)
	{
		auto* thread = get_thread_by_id(thread_id, check_linked);

		if (!thread)
		{
			// Notify the user that the thread is already terminated.
			// (Or wouldn't otherwise conflict with creating a new one)
			return true;
		}

		return static_cast<bool>(stop_thread_impl(*thread));
	}

	bool EntityThreadComponent::stop_thread_by_id
	(
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id,
		bool check_linked
	)
	{
		if (const auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return stop_thread(*thread_index, check_linked);
		}

		return stop_thread_by_id(thread_id, check_linked);
	}

	std::size_t EntityThreadComponent::stop_threads(const EntityThreadRange& thread_range, bool check_linked)
	{
		std::size_t terminated_count = 0;

		return iterate_thread_range
		(
			thread_range,

			[this, check_linked, &terminated_count](EntityThreadIndex thread_index)
			{
				if (stop_thread(thread_index, check_linked))
				{
					terminated_count++;
				}
			}
		);

		return terminated_count;
	}

	std::size_t EntityThreadComponent::stop_threads(EntityStateIndex state_index)
	{
		auto terminated_thread_count = std::size_t {};

		for (auto it = threads.begin(); it != threads.end();)
		{
			auto& thread = *it;

			if (state_index == thread.get_state_index().value_or(state_index))
			{
				if (auto updated_it = stop_thread_impl(it))
				{
					it = *updated_it;

					terminated_thread_count++;

					continue;
				}
			}

			++it;
		}

		return terminated_thread_count;
	}

	std::size_t EntityThreadComponent::stop_threads(const EntityDescriptor& descriptor, EntityStateIndex state_index, bool limit_to_static_range)
	{
		if (limit_to_static_range)
		{
			if (auto* state = descriptor.get_state_by_index(state_index))
			{
				return stop_threads(*state, state_index, limit_to_static_range);
			}
			else
			{
				return 0;
			}
		}

		return stop_threads(state_index);
	}

	std::size_t EntityThreadComponent::stop_threads(const EntityState& state, EntityStateIndex state_index, bool limit_to_static_range)
	{
		if (limit_to_static_range)
		{
			std::size_t terminated_thread_count = 0;

			for (const auto& threads : state.immediate_threads)
			{
				terminated_thread_count += stop_threads(threads, false);
			}

			return terminated_thread_count;
		}
		
		return stop_threads(state_index);
	}

	std::size_t EntityThreadComponent::stop_all()
	{
		std::size_t terminated_thread_count = 0;

		for (auto it = threads.begin(); it != threads.end();)
		{
			if (auto updated_it = stop_thread_impl(it))
			{
				terminated_thread_count++;

				it = *updated_it;
			}
			else
			{
				++it;
			}
		}

		return terminated_thread_count;
	}

	EntityThreadComponent::EntityThreadIterator EntityThreadComponent::erase_thread(EntityThreadIterator thread_it)
	{
		return threads.erase(thread_it);
	}

	bool EntityThreadComponent::erase_thread(EntityThreadIndex thread_index)
	{
		if (auto* thread = get_thread(thread_index, false))
		{
			erase_thread(get_thread_iterator(*thread));
			
			return true;
		}

		// Notify the user that the thread has already been erased.
		return false;
	}

	bool EntityThreadComponent::erase_thread_by_id(EntityThreadID thread_id)
	{
		if (auto* thread = get_thread_by_id(thread_id, false))
		{
			erase_thread(get_thread_iterator(*thread));
			
			return true;
		}

		// Notify the user that the thread has already been erased.
		return false;
	}

	bool EntityThreadComponent::erase_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id)
	{
		if (const auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return erase_thread(*thread_index);
		}

		return erase_thread_by_id(thread_id);
	}

	std::size_t EntityThreadComponent::erase_threads(const EntityThreadRange& thread_range)
	{
		std::size_t erased_count = 0;

		return iterate_thread_range
		(
			thread_range,

			[this, &erased_count](EntityThreadIndex thread_index)
			{
				if (erase_thread(thread_index))
				{
					erased_count++;
				}
			}
		);

		return erased_count;
	}

	std::size_t EntityThreadComponent::erase_threads(EntityStateIndex state_index)
	{
		std::size_t erased_thread_count = 0;

		for (auto it = threads.begin(); it != threads.end();)
		{
			auto& thread = *it;

			if (state_index == thread.get_state_index().value_or(state_index))
			{
				it = threads.erase(it);

				erased_thread_count++;

				continue;
			}

			++it;
		}

		return erased_thread_count;
	}

	std::size_t EntityThreadComponent::erase_threads(const EntityDescriptor& descriptor, EntityStateIndex state_index, bool limit_to_static_range)
	{
		if (limit_to_static_range)
		{
			if (auto* state = descriptor.get_state_by_index(state_index))
			{
				return erase_threads(*state, state_index, limit_to_static_range);
			}
			else
			{
				return 0;
			}
		}

		return erase_threads(state_index);
	}

	std::size_t EntityThreadComponent::erase_threads(const EntityState& state, EntityStateIndex state_index, bool limit_to_static_range)
	{
		if (limit_to_static_range)
		{
			std::size_t terminated_thread_count = 0;

			for (const auto& threads : state.immediate_threads)
			{
				terminated_thread_count += erase_threads(threads);
			}

			return terminated_thread_count;
		}
		
		return erase_threads(state_index);
	}

	std::size_t EntityThreadComponent::erase_completed_threads()
	{
		const auto initial_thread_count = threads.size();

		threads.erase
		(
			std::remove_if
			(
				threads.begin(), threads.end(),

				[this](const EntityThread& thread_entry)
				{
					// If we're not complete, disregard this thread.
					if (!thread_entry.is_complete)
					{
						return false;
					}

					// If we're not linked, don't worry about child threads.
					if (!thread_entry.is_linked)
					{
						return true;
					}

					// When linked, only erase this thread if there are no sub-threads claiming us as their parent:
					const auto child_thread_it = std::find_if
					(
						threads.begin(), threads.end(),

						[&thread_entry](const EntityThread& child_thread)
						{
							return (child_thread.parent_thread_id == thread_entry.thread_id);
						}
					);

					if (child_thread_it != threads.end())
					{
						return false;
					}

					return true;
				}
			),

			threads.end()
		);

		const auto updated_thread_count = threads.size();

		return (initial_thread_count - updated_thread_count);
	}

	bool EntityThreadComponent::pause_thread(EntityThreadIndex thread_index, bool check_linked)
	{
		auto* thread = get_thread(thread_index, check_linked);

		if (!thread)
		{
			return false;
		}

		return thread->pause();
	}

	bool EntityThreadComponent::pause_thread_by_id(EntityThreadID thread_id, bool check_linked)
	{
		auto* thread = get_thread_by_id(thread_id, check_linked);

		if (!thread)
		{
			return false;
		}

		return thread->pause();
	}

	bool EntityThreadComponent::pause_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked)
	{
		if (const auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return pause_thread(*thread_index, check_linked);
		}

		return pause_thread_by_id(thread_id, check_linked);
	}

	std::size_t EntityThreadComponent::pause_threads(const EntityThreadRange& thread_range, bool check_linked)
	{
		std::size_t paused_count = 0;

		return iterate_thread_range
		(
			thread_range,

			[this, check_linked, &paused_count](EntityThreadIndex thread_index)
			{
				if (pause_thread(thread_index, check_linked))
				{
					paused_count++;
				}
			}
		);

		return paused_count;
	}

	bool EntityThreadComponent::resume_thread(EntityThreadIndex thread_index, bool check_linked)
	{
		auto* thread = get_thread(thread_index, check_linked);

		if (!thread)
		{
			return false;
		}

		return thread->resume();
	}

	bool EntityThreadComponent::resume_thread_by_id(EntityThreadID thread_id, bool check_linked)
	{
		auto* thread = get_thread_by_id(thread_id, check_linked);

		if (!thread)
		{
			return false;
		}

		return thread->resume();
	}

	bool EntityThreadComponent::resume_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked)
	{
		if (const auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return resume_thread(*thread_index, check_linked);
		}

		return resume_thread_by_id(thread_id, check_linked);
	}

	std::size_t EntityThreadComponent::resume_threads(const EntityThreadRange& thread_range, bool check_linked)
	{
		std::size_t resumed_count = 0;

		return iterate_thread_range
		(
			thread_range,

			[this, check_linked, &resumed_count](EntityThreadIndex thread_index)
			{
				if (resume_thread(thread_index, check_linked))
				{
					resumed_count++;
				}
			}
		);

		return resumed_count;
	}

	bool EntityThreadComponent::attach_thread
	(
		EntityThreadIndex thread_index,
		std::optional<EntityStateIndex> state_index,
		bool check_linked
	)
	{
		auto* thread = get_thread(thread_index, check_linked);

		if (!thread)
		{
			return false;
		}

		return thread->attach(state_index);
	}

	bool EntityThreadComponent::attach_thread_by_id
	(
		EntityThreadID thread_id,
		std::optional<EntityStateIndex> state_index,
		bool check_linked
	)
	{
		auto* thread = get_thread_by_id(thread_id, check_linked);

		if (!thread)
		{
			return false;
		}

		return thread->attach(state_index);
	}

	bool EntityThreadComponent::attach_thread_by_id
	(
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id,
		std::optional<EntityStateIndex> state_index,
		bool check_linked
	)
	{
		if (const auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return attach_thread(*thread_index, state_index, check_linked);
		}

		return attach_thread_by_id(thread_id, state_index, check_linked);
	}

	std::size_t EntityThreadComponent::attach_threads
	(
		const EntityThreadRange& thread_range,
		std::optional<EntityStateIndex> state_index,
		bool check_linked
	)
	{
		std::size_t attached_count = 0;

		return iterate_thread_range
		(
			thread_range,

			[this, check_linked, state_index, &attached_count](EntityThreadIndex thread_index)
			{
				if (attach_thread(thread_index, state_index, check_linked))
				{
					attached_count++;
				}
			}
		);

		return attached_count;
	}

	bool EntityThreadComponent::detach_thread(EntityThreadIndex thread_index, bool check_linked)
	{
		auto* thread = get_thread(thread_index, check_linked);

		if (!thread)
		{
			return false;
		}

		return thread->detach();
	}

	bool EntityThreadComponent::detach_thread_by_id(EntityThreadID thread_id, bool check_linked)
	{
		auto* thread = get_thread_by_id(thread_id, check_linked);

		if (!thread)
		{
			return false;
		}

		return thread->detach();
	}

	bool EntityThreadComponent::detach_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked)
	{
		if (const auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return detach_thread(*thread_index, check_linked);
		}

		return detach_thread_by_id(thread_id, check_linked);
	}

	std::size_t EntityThreadComponent::detach_threads(const EntityThreadRange& thread_range, bool check_linked)
	{
		std::size_t detached_count = 0;

		return iterate_thread_range
		(
			thread_range,

			[this, check_linked, &detached_count](EntityThreadIndex thread_index)
			{
				if (detach_thread(thread_index, check_linked))
				{
					detached_count++;
				}
			}
		);

		return detached_count;
	}

	bool EntityThreadComponent::unlink_thread(EntityThreadIndex thread_index)
	{
		auto* thread = get_thread(thread_index, true);

		if (!thread)
		{
			return false;
		}

		return thread->unlink();
	}

	bool EntityThreadComponent::unlink_thread_by_id(EntityThreadID thread_id)
	{
		auto* thread = get_thread_by_id(thread_id, true);

		if (!thread)
		{
			return false;
		}

		return thread->unlink();
	}

	bool EntityThreadComponent::unlink_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id)
	{
		if (const auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return unlink_thread(*thread_index);
		}

		return unlink_thread_by_id(thread_id);
	}

	std::size_t EntityThreadComponent::unlink_threads(const EntityThreadRange& thread_range)
	{
		std::size_t unlinked_count = 0;

		return iterate_thread_range
		(
			thread_range,

			[this, &unlinked_count](EntityThreadIndex thread_index)
			{
				if (unlink_thread(thread_index))
				{
					unlinked_count++;
				}
			}
		);

		return unlinked_count;
	}

	bool EntityThreadComponent::link_local_thread(LocalThreadIndex local_thread_index)
	{
		auto& thread = threads.at(local_thread_index); // threads[local_thread_index];

		if (thread.is_linked)
		{
			return true;
		}

		// Look for an existing linked thread using the same underlying index.
		if (get_thread(thread.thread_index, true))
		{
			return false;
		}

		thread.is_linked = true;

		return true;
	}

	bool EntityThreadComponent::skip_thread(EntityThreadIndex thread_index, EntityInstructionCount instruction_count, bool check_linked)
	{
		auto* thread = get_thread(thread_index, check_linked);

		if (!thread)
		{
			return false;
		}

		return (thread->skip(instruction_count) > 0); // == instruction_count
	}

	bool EntityThreadComponent::skip_thread_by_id(EntityThreadID thread_id, EntityInstructionCount instruction_count, bool check_linked)
	{
		auto* thread = get_thread_by_id(thread_id, check_linked);

		if (!thread)
		{
			return false;
		}

		return (thread->skip(instruction_count) > 0); // == instruction_count
	}

	bool EntityThreadComponent::skip_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, EntityInstructionCount instruction_count, bool check_linked)
	{
		if (const auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return skip_thread(*thread_index, instruction_count, check_linked);
		}

		return skip_thread_by_id(thread_id, instruction_count, check_linked);
	}

	std::size_t EntityThreadComponent::skip_threads(const EntityThreadRange& thread_range, EntityInstructionCount instruction_count, bool check_linked)
	{
		std::size_t skipped_count = 0;

		return iterate_thread_range
		(
			thread_range,

			[this, instruction_count, check_linked, &skipped_count](EntityThreadIndex thread_index)
			{
				if (skip_thread(thread_index, instruction_count, check_linked))
				{
					skipped_count++;
				}
			}
		);

		return skipped_count;
	}

	bool EntityThreadComponent::rewind_thread
	(
		EntityThreadIndex thread_index,
		EntityInstructionCount instruction_count,
		bool check_linked
	)
	{
		auto* thread = get_thread(thread_index, check_linked);

		if (!thread)
		{
			return false;
		}

		return (thread->rewind(instruction_count) > 0); // == instruction_count
	}

	bool EntityThreadComponent::rewind_thread_by_id(EntityThreadID thread_id, EntityInstructionCount instruction_count, bool check_linked)
	{
		auto* thread = get_thread_by_id(thread_id, check_linked);

		if (!thread)
		{
			return false;
		}

		return (thread->rewind(instruction_count) > 0); // == instruction_count
	}

	bool EntityThreadComponent::rewind_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, EntityInstructionCount instruction_count, bool check_linked)
	{
		if (const auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return rewind_thread(*thread_index, instruction_count, check_linked);
		}

		return rewind_thread_by_id(thread_id, instruction_count, check_linked);
	}

	std::size_t EntityThreadComponent::rewind_threads(const EntityThreadRange& thread_range, EntityInstructionCount instruction_count, bool check_linked)
	{
		std::size_t rewound_count = 0;

		return iterate_thread_range
		(
			thread_range,

			[this, instruction_count, check_linked, &rewound_count](EntityThreadIndex thread_index)
			{
				if (rewind_thread(thread_index, instruction_count, check_linked))
				{
					rewound_count++;
				}
			}
		);

		return rewound_count;
	}

	bool EntityThreadComponent::is_thread_running(EntityThreadIndex thread_index) const
	{
		return static_cast<bool>(get_thread(thread_index, true));
	}

	bool EntityThreadComponent::is_thread_with_id_running(EntityThreadID thread_id) const
	{
		return static_cast<bool>(get_thread_by_id(thread_id, true));
	}

	bool EntityThreadComponent::is_thread_with_id_running(const EntityDescriptor& descriptor, EntityThreadID thread_id) const
	{
		return static_cast<bool>(get_thread_by_id(descriptor, thread_id, true));
	}

	const EntityThread* EntityThreadComponent::get_thread(EntityThreadIndex thread_index, bool check_linked) const
	{
		if (thread_index == ENTITY_THREAD_INDEX_INVALID)
		{
			return {};
		}

		for (const auto& thread : threads)
		{
			if (thread.thread_index == thread_index)
			{
				if (!check_linked || (thread.is_linked))
				{
					return &thread;
				}
			}
		}

		return {};
	}

	const EntityThread* EntityThreadComponent::get_thread_by_id(EntityThreadID thread_id, bool check_linked) const
	{
		if (!thread_id)
		{
			return {};
		}

		for (const auto& thread : threads)
		{
			if (thread.thread_id == thread_id)
			{
				if (!check_linked || (thread.is_linked))
				{
					return &thread;
				}
			}
		}

		return {};
	}

	const EntityThread* EntityThreadComponent::get_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked) const
	{
		if (!thread_id)
		{
			return {};
		}

		if (const auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return get_thread(*thread_index, check_linked);
		}

		if (const auto thread = get_thread_by_id(thread_id, check_linked))
		{
			return thread;
		}

		return {};
	}

	// Retrieves an iterator for a given thread.
	EntityThreadComponent::EntityThreadIterator EntityThreadComponent::get_thread_iterator(const EntityThread& thread)
	{
		const auto thread_ptr = &thread;

		if ((thread_ptr >= threads.data()) && (thread_ptr < (threads.data() + threads.size())))
		{
			return (threads.begin() + (thread_ptr - threads.data()));
		}

		return threads.end();
	}

	void EntityThreadComponent::terminate_all()
	{
		threads.clear();
	}

	std::optional<EntityThreadComponent::LocalThreadIndex> EntityThreadComponent::get_local_index(const EntityThread& thread) const
	{
		// Alternative implementation:
		/*
		const auto thread_it = &thread;

		if ((thread_it >= threads.cbegin()) && (thread_it < threads.cend()))
		{
			return static_cast<LocalThreadIndex>(std::distance(threads.cbegin(), thread_it));
		}
		*/

		const auto thread_ptr = &thread;

		if ((thread_ptr >= threads.data()) && (thread_ptr < (threads.data() + threads.size())))
		{
			return static_cast<LocalThreadIndex>(std::distance(threads.data(), thread_ptr));
		}

		return std::nullopt;
	}

	std::optional<EntityThreadComponent::EntityThreadIterator> EntityThreadComponent::stop_thread_impl(EntityThread& thread)
	{
		if (auto thread_it = get_thread_iterator(thread); thread_it != threads.end())
		{
			return stop_thread_impl(thread_it);
		}

		return std::nullopt;
	}

	std::optional<EntityThreadComponent::EntityThreadIterator> EntityThreadComponent::stop_thread_impl(EntityThreadIterator thread_it)
	{
		if (thread_it == threads.end())
		{
			return threads.end();
		}

		auto& thread = *thread_it;

		if (thread.is_detached)
		{
			// If this is a detached thread, make sure it's unlinked.
			thread.is_linked = false;

			// NOTE: Detached threads remain in the `threads` container, but do not
			// stop the creation of another with the same ID or index.

			// Indicate that we detached the thread, rather than terminated it.
			return {};
		}

		// Indicate that we terminated the thread.
		return threads.erase(thread_it);
	}
}