#pragma once

//#include "entity_state.hpp"

#include "types.hpp"
#include "entity_descriptor_shared.hpp"

#include <util/small_vector.hpp>

namespace engine
{
	class EntityState;

	// TODO: Implement multi-entity support for state descriptions via secondary storage mechanism.
	struct EntityStateCollection
	{
		using ElementType = EntityDescriptorShared<EntityState>;

		// Could also be handled as a map-type, but that would be
		// less efficient for what is normally a small number of states.
		using CollectionType = util::small_vector<ElementType, 16>; // 32 // util::small_vector<EntityState, 4>; // std::shared_ptr<...>;

		const EntityState* get_state(const auto& storage, EntityStateID name) const
		{
			for (const auto& state_entry : data)
			{
				const auto& state = state_entry.get(storage);

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

		EntityState* get_state(auto& storage, EntityStateID name)
		{
			return const_cast<EntityState*>(const_cast<const EntityStateCollection*>(this)->get_state(storage, name));
		}

		const EntityState* get_state(const auto& storage, std::optional<EntityStateID> name) const
		{
			if (!name)
			{
				return {};
			}

			return get_state(storage, *name);
		}

		EntityState* get_state(auto& storage, std::optional<EntityStateID> name)
		{
			if (!name)
			{
				return {};
			}

			return get_state(storage, *name);
		}

		std::optional<EntityStateIndex> get_state_index(const auto& storage, EntityStateID name) const // std::size_t
		{
			for (EntityStateIndex i = 0; i < data.size(); i++)
			{
				const auto& state_entry = data[i];
				const auto& state = state_entry.get(storage);

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

		std::optional<EntityStateIndex> get_state_index(const auto& storage, std::optional<EntityStateID> name) const
		{
			if (!name)
			{
				return std::nullopt;
			}

			return get_state_index(storage, *name);
		}

		const EntityState* get_state_by_index(const auto& storage, EntityStateIndex state_index) const
		{
			const auto state_index_promoted = static_cast<std::size_t>(state_index);

			assert(state_index_promoted < data.size());

			const auto& state_entry = data[state_index_promoted];

			return &(state_entry.get(storage));
		}

		inline std::size_t std_size() const
		{
			return data.size();
		}

		inline EntityStateCount size() const
		{
			return static_cast<EntityStateCount>(std_size());
		}

		inline const ElementType& operator[](std::size_t index) const
		{
			return data[index];
		}

		inline const ElementType& operator[](EntityStateIndex index) const
		{
			return operator[](static_cast<std::size_t>(index));
		}

		inline ElementType& operator[](std::size_t index)
		{
			return data[index];
		}

		inline ElementType& operator[](EntityStateIndex index)
		{
			return operator[](static_cast<std::size_t>(index));
		}

		CollectionType data;
	};
}