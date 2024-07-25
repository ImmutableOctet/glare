#include "reflection.hpp"

#include "player_system.hpp"

#include "components/player_component.hpp"
#include "components/player_flags_component.hpp"
#include "components/player_name_component.hpp"
#include "components/player_target_component.hpp"

namespace engine
{
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(PlayerTargetComponent, player_index);
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(PlayerComponent,       player_index);
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(PlayerNameComponent,   name);
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(PlayerFlagsComponent,  flags);

	template <>
	void reflect<PlayerSystem>()
	{
		auto player_system = engine_system_type<PlayerSystem>()
			// ...
		;

		reflect<PlayerFlags>();

		reflect<PlayerTargetComponent>();

		reflect<PlayerComponent>();
		reflect<PlayerNameComponent>();
		reflect<PlayerFlagsComponent>();
	}
}