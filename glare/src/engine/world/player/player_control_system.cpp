#include "player_control_system.hpp"
#include "player_control_component.hpp"

#include <engine/world/world.hpp>

#include <app/input/events.hpp>
#include <app/input/gamepad_state.hpp>

namespace engine
{
	PlayerControlSystem::PlayerControlSystem(World& world)
		: WorldSystem(world) {}

	void PlayerControlSystem::on_subscribe(World& world)
	{
		//world.register_event<>(*this);
	}

	void PlayerControlSystem::on_update(World& world, float delta)
	{

	}
}