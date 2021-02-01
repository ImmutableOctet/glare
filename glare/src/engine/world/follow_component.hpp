#pragma once

#include "entity.hpp"

namespace engine
{
	struct SimpleFollowComponent
	{
		static void update(World& world);

		static constexpr float DEFAULT_FOLLOWING_DISTANCE = 5.0f;
		static constexpr float DEFAULT_FOLLOW_SPEED = 0.1f;
		static constexpr float DEFAULT_MAX_DISTANCE = 200.0f;
		static constexpr bool  DEFAULT_FORCE_CATCH_UP = true;

		// The Entity being followed.
		Entity leader = null;

		// The ideal distance between this Entity and the leader.
		float following_distance = DEFAULT_FOLLOWING_DISTANCE;
		float follow_speed = DEFAULT_FOLLOW_SPEED;
		float max_distance = DEFAULT_MAX_DISTANCE;

		/*
			When enabled, if the distance between this entity and the leader is > 'max_distance',
			this will force this entity to catch up to the leader. (teleports to 'following_distance' from leader)

			When disabled, if the distance to the leader is > 'max_distance',
			the 'following' flag will be disabled until the leader is back in range.
		*/
		bool force_catch_up = DEFAULT_FORCE_CATCH_UP;

		// If enabled, this entity will follow the leader.
		bool following = true;

		void apply(World& world, Entity entity, Transform& transform);
	};
}