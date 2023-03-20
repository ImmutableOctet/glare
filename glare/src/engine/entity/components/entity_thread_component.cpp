#include "entity_thread_component.hpp"

#include <engine/entity/entity_thread.hpp>
#include <engine/entity/entity_thread_description.hpp>
#include <engine/entity/entity_descriptor.hpp>
#include <engine/entity/entity_state.hpp>

#include <iterator>

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

	EntityThread* EntityThreadComponent::start_thread
	(
		EntityThreadIndex thread_index,

		std::optional<EntityStateIndex> state_index,
		bool check_existing,
		bool check_linked,
		bool restart_existing,
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
						0,
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
			0,
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
		const EntityThreadFlags& flags
	)
	{
		if (auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return start_thread
			(
				*thread_index, state_index,
				check_existing, check_linked, restart_existing,
				flags
			);
		}

		return {};
	}

	std::size_t EntityThreadComponent::start_threads
	(
		const EntityThreadRange& thread_range,
		
		std::optional<EntityStateIndex> state_index,
		bool restart_existing
	)
	{
		std::size_t started_thread_count = 0;

		iterate_thread_range
		(
			thread_range,
			[this, &state_index, restart_existing, &started_thread_count](EntityThreadIndex thread_index)
			{
				if (start_thread(thread_index, state_index, true, true, restart_existing))
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
		bool restart_existing
	)
	{
		const auto* state = descriptor.get_state_by_index(state_index);

		if (!state)
		{
			return 0;
		}

		return start_threads(*state, state_index, restart_existing);
	}

	std::size_t EntityThreadComponent::start_threads
	(
		const EntityState& state,
		EntityStateIndex state_index,
		bool restart_existing
	)
	{
		std::size_t threads_started = 0;

		for (const auto& threads : state.immediate_threads)
		{
			threads_started += start_threads(threads, state_index, restart_existing);
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
			// (Or wouldn't otherwise conflict creating a new one)
			return true;
		}

		return static_cast<bool>(stop_thread_impl(*thread));
	}

	bool EntityThreadComponent::stop_thread
	(
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id,
		bool check_linked
	)
	{
		if (auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return stop_thread(*thread_index, check_linked);
		}

		return true; // false;
	}

	// The return value of this method indicates how many threads were fully terminated.
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
		std::size_t terminated_thread_count = 0;

		for (auto it = threads.begin(); it != threads.end();)
		{
			auto& thread = *it;

			if (state_index == thread.state_index.value_or(state_index))
			{
				if (auto updated_it = stop_thread_impl(thread))
				{
					it = updated_it;

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

	// Attempts to stop all threads.
	// 
	// The return value of this method indicates how many threads were fully terminated.
	std::size_t EntityThreadComponent::stop_all()
	{
		std::size_t terminated_thread_count = 0;

		for (auto it = threads.begin(); it != threads.end();)
		{
			if (auto updated_it = stop_thread_impl(*it))
			{
				it = updated_it;

				terminated_thread_count++;
			}
			else
			{
				++it;
			}
		}

		return terminated_thread_count;
	}

	bool EntityThreadComponent::pause_thread
	(
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id,
		bool check_linked
	)
	{
		if (auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return pause_thread(*thread_index, check_linked);
		}

		return false;
	}

	bool EntityThreadComponent::pause_thread
	(
		EntityThreadIndex thread_index,
		bool check_linked
	)
	{
		auto* thread = get_thread(thread_index, check_linked);

		if (!thread)
		{
			return false;
		}

		return thread->pause();
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

	bool EntityThreadComponent::resume_thread
	(
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id,
		bool check_linked
	)
	{
		if (auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return resume_thread(*thread_index, check_linked);
		}

		return false;
	}

	bool EntityThreadComponent::resume_thread
	(
		EntityThreadIndex thread_index,
		bool check_linked
	)
	{
		auto* thread = get_thread(thread_index, check_linked);

		if (!thread)
		{
			return false;
		}

		return thread->resume();
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
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id,
		std::optional<EntityStateIndex> state_index,
		bool check_linked
	)
	{
		if (auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return attach_thread(*thread_index, state_index, check_linked);
		}

		return false;
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

	bool EntityThreadComponent::detach_thread
	(
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id,
		bool check_linked
	)
	{
		if (auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return detach_thread(*thread_index, check_linked);
		}

		return false;
	}

	bool EntityThreadComponent::detach_thread
	(
		EntityThreadIndex thread_index,
		bool check_linked
	)
	{
		auto* thread = get_thread(thread_index, check_linked);

		if (!thread)
		{
			return false;
		}

		return thread->detach();
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

	bool EntityThreadComponent::unlink_thread
	(
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id
	)
	{
		if (auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return unlink_thread(*thread_index);
		}

		return false;
	}

	bool EntityThreadComponent::unlink_thread
	(
		EntityThreadIndex thread_index
	)
	{
		auto* thread = get_thread(thread_index, true);

		if (!thread)
		{
			return false;
		}

		return thread->unlink();
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

	bool EntityThreadComponent::link_thread(LocalThreadIndex local_thread_index)
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

	bool EntityThreadComponent::skip_thread
	(
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id,
		EntityInstructionCount instruction_count,
		bool check_linked
	)
	{
		if (auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return skip_thread(*thread_index, instruction_count, check_linked);
		}

		return false;
	}

	bool EntityThreadComponent::skip_thread
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

		return (thread->skip(instruction_count) > 0); // == instruction_count
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
		const EntityDescriptor& descriptor,
		EntityThreadID thread_id,
		EntityInstructionCount instruction_count,
		bool check_linked
	)
	{
		if (auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return rewind_thread(*thread_index, instruction_count, check_linked);
		}

		return false;
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

	bool EntityThreadComponent::thread_running(EntityThreadIndex thread_index) const
	{
		return static_cast<bool>(get_thread(thread_index, true));
	}

	bool EntityThreadComponent::thread_running(const EntityDescriptor& descriptor, EntityThreadID thread_id) const
	{
		return static_cast<bool>(get_thread(descriptor, thread_id, true));
	}

	const EntityThread* EntityThreadComponent::get_thread(EntityThreadIndex thread_index, bool check_linked) const
	{
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

	const EntityThread* EntityThreadComponent::get_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked) const
	{
		if (auto thread_index = descriptor.get_thread_index(thread_id))
		{
			return get_thread(*thread_index, check_linked);
		}

		return {};
	}

	void EntityThreadComponent::terminate_all()
	{
		threads.clear();
	}

	std::optional<EntityThreadComponent::LocalThreadIndex> EntityThreadComponent::get_local_index(const EntityThread& thread) const
	{
		//return static_cast<LocalThreadIndex>(&thread - threads.data());

		const auto* thread_it = &thread;

		if ((thread_it >= threads.cbegin()) && (thread_it < threads.cend()))
		{
			return static_cast<LocalThreadIndex>(std::distance(threads.cbegin(), thread_it));
		}

		return std::nullopt;
	}

	EntityThread* EntityThreadComponent::stop_thread_impl(EntityThread& thread)
	{
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
		return threads.erase(&thread);
	}
}