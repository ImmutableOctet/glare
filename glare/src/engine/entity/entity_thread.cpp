#include "entity_thread.hpp"

//#include <util/variant.hpp>

namespace engine
{
	EntityThread::EntityThread
	(
		const EntityThreadFlags& flags,
		EntityThreadIndex thread_index,
		InstructionIndex first_instruction,
		std::optional<EntityStateIndex> state_index
	) :
		EntityThreadFlags(flags),
		thread_index(thread_index),
		next_instruction(first_instruction),
		state_index(state_index)
	{}

	bool EntityThread::pause()
	{
		const auto was_paused = is_paused;

		is_paused = true;

		return !was_paused;
	}

	bool EntityThread::resume()
	{
		const auto was_paused = is_paused;

		is_paused = false;

		return was_paused;
	}

	bool EntityThread::link()
	{
		const auto was_linked = is_linked;

		is_linked = true;

		return !was_linked;
	}

	bool EntityThread::unlink()
	{
		const auto was_linked = is_linked;

		is_linked = false;

		return was_linked;
	}

	bool EntityThread::attach(std::optional<EntityStateIndex> state_index, bool keep_existing_state)
	{
		const auto was_detached = is_detached;

		is_detached = false;

		if (state_index)
		{
			if (!keep_existing_state || !this->state_index.has_value())
			{
				this->state_index = *state_index;
			}
		}

		return was_detached;
	}

	bool EntityThread::detach()
	{
		const auto was_detached = is_detached;

		is_detached = true;

		return !was_detached;
	}

	EntityThread::ThreadLocalVariables* EntityThread::get_variables()
	{
		if (!variables)
		{
			variables = std::make_shared<ThreadLocalVariables>();
		}

		return variables.get();
	}

	EntityInstructionCount EntityThread::skip(EntityInstructionCount forward_stride)
	{
		next_instruction += forward_stride;

		return forward_stride;
	}

	EntityInstructionCount EntityThread::rewind(EntityInstructionCount backward_stride)
	{
		// Ensure we can actually move back the amount requested.
		assert(next_instruction >= backward_stride);

		next_instruction -= backward_stride;

		return backward_stride;
	}
}