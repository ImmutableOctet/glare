#include "physics_component.hpp"

#include <engine/world/world.hpp>

namespace engine
{
	PhysicsComponent::PhysicsComponent(PhysicsComponent::Flags motion_flags, MotionState motion_state)
		: flags(motion_flags), motion(motion_state)
	{}

	Entity attach_physics(World& world, Entity entity, PhysicsComponent::Flags flags, PhysicsComponent::MotionState motion_state)
	{
		auto& registry = world.get_registry();

		registry.emplace_or_replace<PhysicsComponent>(entity, flags, motion_state);

		return entity;
	}
}