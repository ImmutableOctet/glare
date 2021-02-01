#pragma once

#include "entity.hpp"

namespace engine
{
	struct SimpleFollowComponent
	{
		static void update(World& world);

		// The Entity being followed.
		Entity leader;

		// The ideal distance between this Entity and the leader.
		float following_distance = 5.0f;
		float follow_speed = 0.1f;
		float max_distance = 200.0f;

		// If enabled, this entity will follow the leader.
		bool following = true;

		/*
			When enabled, if the distance between this entity and the leader is > 'max_distance',
			this will force this entity to catch up to the leader. (teleports to 'following_distance' from leader)

			When disabled, if the distance to the leader is > 'max_distance',
			the 'following' flag will be disabled until the leader is back in range.
		*/
		bool force_catch_up = true;

		void apply(World& world, Entity entity, Transform& transform);
	};
}