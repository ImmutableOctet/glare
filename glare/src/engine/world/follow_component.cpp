#include "follow_component.hpp"

#include "world.hpp"
#include "physics/physics_component.hpp"

#include <math/math.hpp>
#include <cmath>

namespace engine
{
	void SimpleFollowComponent::update(World& world)
	{
		auto& registry = world.get_registry();

		registry.view<SimpleFollowComponent>().each([&](auto entity, auto& follow_comp) // TransformComponent, ...
		{
			auto transform = world.get_transform(entity);

			follow_comp.apply(world, entity, transform);
		});
	}

	void SimpleFollowComponent::apply(World& world, Entity entity, Transform& transform)
	{
		if (leader == null)
		{
			return;
		}

		auto delta = world.delta();
		auto& registry = world.get_registry();

		auto leader_transform = world.get_transform(leader);

		auto pos = transform.get_position();
		auto leader_pos = leader_transform.get_position();

		auto position_delta = (leader_pos - pos);

		auto distance = glm::length(position_delta);
		auto direction = glm::normalize(position_delta);

		if (distance > max_distance)
		{
			if (force_catch_up)
			{
				// Teleport this entity to the appropriate distance from the leader.
				transform.set_position(leader_pos + (following_distance * -direction));
			}
			else
			{
				following = false;
			}
		}
		else if (!following)
		{
			// Begin following on next update.
			following = true;

			// TODO: Add event for 'OnBeginFollow'.

			return;
		}

		if (!following)
		{
			return;
		}

		auto movement_vector = (direction * std::min(std::min(follow_speed, distance), following_distance));

		auto* physics = registry.try_get<PhysicsComponent>(entity);

		if (distance > following_distance)
		{
			if (physics)
			{
				physics->motion.velocity = movement_vector;
			}
			else
			{
				//transform.set_position(pos + (movement_vector * delta));
				transform.move((movement_vector * delta));

				//transform.set_position(pos);
				//transform.set_local_position(pos);
			}
		}
		else
		{
			if (physics)
			{
				physics->motion.velocity = {};
			}
		}
	}
}