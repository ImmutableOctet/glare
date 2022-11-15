#include "entity_descriptor.hpp"
#include "meta/meta.hpp"
#include "state/components/state_component.hpp"

namespace engine
{
	const EntityState* EntityDescriptor::get_state(StringHash name) const
	{
		for (const auto& state : states)
		{
			if (!state.name)
			{
				continue;
			}

			if ((*state.name) == name)
			{
				return &state;
			}
		}

		return nullptr;
	}

	EntityState* EntityDescriptor::get_state(StringHash name)
	{
		return const_cast<EntityState*>(const_cast<const EntityDescriptor*>(this)->get_state(name));
	}

	std::optional<EntityStateIndex> EntityDescriptor::get_state_index(StringHash name) const // std::size_t
	{
		for (EntityStateIndex i = 0; i < states.size(); i++)
		{
			const auto& state = states[i];

			if (!state.name)
			{
				continue;
			}

			if ((*state.name) == name)
			{
				return i;
			}
		}

		return std::nullopt;
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

			from = &(states[*previous_state]);
		}

		const auto& to = states[state_index];

		to.update(registry, entity, from);

		registry.emplace_or_replace<StateComponent>(entity, state_index);

		return true;
	}
}