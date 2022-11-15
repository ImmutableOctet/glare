#pragma once

#include <engine/types.hpp>

namespace engine
{
	class World;
	struct Transform;

	// Applies a simple following behavior, moving an
	// entity towards a targeted `leader` entity.
	struct SimpleFollowBehavior
	{
		static void on_update(World& world, float delta);

		static constexpr float DEFAULT_FOLLOWING_DISTANCE = 5.0f;
		static constexpr float DEFAULT_FOLLOW_SPEED       = 0.1f;
		static constexpr float DEFAULT_MAX_DISTANCE       = 200.0f;
		static constexpr bool  DEFAULT_FORCE_CATCH_UP     = true;

		// The Entity being followed.
		Entity leader = null;

		// The ideal distance between this Entity and the leader.
		float following_distance = DEFAULT_FOLLOWING_DISTANCE;
		float follow_speed       = DEFAULT_FOLLOW_SPEED;
		float max_distance       = DEFAULT_MAX_DISTANCE;

		/*
			When enabled, if the distance between this entity and the leader is > 'max_distance',
			this will force this entity to catch up to the leader. (teleports to 'following_distance' from leader)

			When disabled, if the distance to the leader is > 'max_distance',
			the 'following' flag will be disabled until the leader is back in range.
		*/
		bool force_catch_up : 1 = DEFAULT_FORCE_CATCH_UP;

		// If enabled, this entity will follow the leader.
		bool following      : 1 = true;

		inline bool get_force_catch_up() const { return force_catch_up; }
		inline void set_force_catch_up(bool value) { force_catch_up = value; }

		inline bool get_following() const { return following; }
		inline void set_following(bool value) { following = value; }
		
		void apply(World& world, Entity entity, Transform& transform, float delta);
	};
}