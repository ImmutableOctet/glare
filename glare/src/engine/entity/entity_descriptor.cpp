#include "entity_descriptor.hpp"

#include <engine/meta/meta.hpp>

namespace engine
{
	// EntityDescriptor:
	MetaTypeDescriptor& EntityDescriptor::generate_empty_command(const MetaType& command_type, Entity source, Entity target)
	{
		using namespace entt::literals;

		//auto command_content = CommandContent(command_type);
		auto& command_content = shared_storage.allocate<MetaTypeDescriptor>(command_type);

		command_content.set_variable(MetaVariable("source"_hs, Entity(null)));
		command_content.set_variable(MetaVariable("target"_hs, Entity(null)));

		return command_content;
	}

	const EntityState* EntityDescriptor::get_state(StringHash name) const
	{
		for (const auto& state : states)
		{
			if (!state->name)
			{
				continue;
			}

			if ((*state->name) == name)
			{
				//return &state;
				return state.get();
			}
		}

		return nullptr;
	}

	EntityState* EntityDescriptor::get_state(StringHash name)
	{
		return const_cast<EntityState*>(const_cast<const EntityDescriptor*>(this)->get_state(name));
	}

	const EntityState* EntityDescriptor::get_state(std::optional<StringHash> name) const
	{
		if (!name)
		{
			return {};
		}

		return get_state(*name);
	}

	EntityState* EntityDescriptor::get_state(std::optional<StringHash> name)
	{
		if (!name)
		{
			return {};
		}

		return get_state(*name);
	}

	std::optional<EntityStateIndex> EntityDescriptor::get_state_index(EntityStateID name) const // std::size_t
	{
		for (EntityStateIndex i = 0; i < states.size(); i++)
		{
			const auto& state = states[i];

			if (!state->name)
			{
				continue;
			}

			if ((*state->name) == name)
			{
				return i;
			}
		}

		return std::nullopt;
	}

	const EntityState* EntityDescriptor::get_state_by_index(EntityStateIndex state_index) const
	{
		const auto state_index_promoted = static_cast<std::size_t>(state_index);

		assert(state_index_promoted < states.size());

		if (const auto& state = states[state_index_promoted])
		{
			if (state)
			{
				return state.get();
			}
		}

		return {};
	}

	bool EntityDescriptor::set_state(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, std::string_view state_name) const
	{
		return set_state_by_id(registry, entity, previous_state, hash(state_name));
	}

	bool EntityDescriptor::set_state_by_id(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, StringHash state_id) const
	{
		auto state_index = get_state_index(state_id);

		if (!state_index)
		{
			return false;
		}

		return set_state_by_index(registry, entity, previous_state, *state_index);
	}

	bool EntityDescriptor::set_state_by_index(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, EntityStateIndex state_index) const
	{
		if (state_index >= states.size())
		{
			return false;
		}

		const EntityState* from = nullptr;

		if (previous_state)
		{
			if ((*previous_state) >= states.size())
			{
				return false;
			}

			from = (states[*previous_state].get()); // &(states[*previous_state]);
		}

		const auto& to = states[state_index];

		to->update(*this, registry, entity, state_index, from, previous_state);

		return true;
	}

	const EntityThreadDescription& EntityDescriptor::get_thread(EntityThreadIndex thread_index) const
	{
		const auto& threads = get_threads();

		return threads[thread_index]; // .at(thread_index);
	}

	// TODO: Optimize via map lookup, etc.
	std::optional<EntityThreadIndex> EntityDescriptor::get_thread_index(EntityThreadID thread_id) const
	{
		const auto& threads = get_threads();

		//for (const auto& thread : threads)
		for (std::size_t i = 0; i < threads.size(); i++)
		{
			const auto& thread = threads[i];

			if (thread.thread_id == thread_id)
			{
				return static_cast<EntityThreadIndex>(i);
			}
		}

		return std::nullopt;
	}

	std::optional<EntityThreadID> EntityDescriptor::get_thread_id(EntityThreadIndex thread_index) const
	{
		const auto& thread = get_thread(thread_index);

		return thread.thread_id;
	}
}