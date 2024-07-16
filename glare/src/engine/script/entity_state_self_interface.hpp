#pragma once

#include <engine/types.hpp>

#include <engine/entity/types.hpp>
#include <engine/entity/entity_state.hpp>
#include <engine/entity/entity_system.hpp>
#include <engine/entity/commands/state_change_command.hpp>

//#include <cassert>

namespace engine
{
	class EntityStateSelfInterface
	{
		public:
			const EntityState* get_state_description(this auto&& self)
			{
				return self.template system<EntitySystem>().get_state(self.get_entity());
			}

			const EntityState* get_prev_state_description(this auto&& self)
			{
				return self.template system<EntitySystem>().get_prev_state(self.get_entity());
			}

			EntityStateID get_state_id(this auto&& self)
			{
				if (const auto state = self.get_state_description())
				{
					if (state->name)
					{
						return *(state->name);
					}
				}

				return {};
			}

			EntityStateID get_prev_state_id(this auto&& self)
			{
				if (const auto prev_state = self.get_prev_state_description())
				{
					if (prev_state->name)
					{
						return *(prev_state->name);
					}
				}

				return {};
			}

			EntityStateID get_state(this auto&& self)
			{
				return self.get_state_id();
			}

			EntityStateID get_prev_state(this auto&& self)
			{
				return self.get_prev_state_id();
			}

			decltype(auto) set_state_id(this auto&& self, EntityStateID state_id)
			{
				return self.template event<StateChangeCommand>
				(
					self.get_entity(),
					self.get_entity(),

					state_id
				);
			}

			decltype(auto) set_state(this auto&& self, const EntityState& state)
			{
				const auto state_id = (state.name)
					? (*state.name)
					: EntityStateID {}
				;

				//assert(state_id);

				return self.set_state_id(state_id);
			}

			decltype(auto) set_state(this auto&& self, EntityStateID state_id)
			{
				return self.set_state_id(state_id);
			}
	};
}