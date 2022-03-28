#include "player.hpp"
#include "world.hpp"
#include "physics.hpp"

#include <util/string.hpp>

#include <engine/resource_manager/resource_manager.hpp>
#include <engine/name_component.hpp>
#include <engine/collision.hpp>

//#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet/btBulletCollisionCommon.h>

#include <algorithm>
#include <string>

// Debugging related:
#include <iostream>

#include <engine/model_component.hpp>

namespace engine
{
	const char* DEFAULT_PLAYER_NAME = "Player";

    Entity create_player(World& world, const math::TransformVectors& tform, Character character, const std::string& name, Entity parent, PlayerIndex index)
    {
		auto [position, rotation, scale] = tform;

		auto& registry = world.get_registry();

		std::string model_path;

		float c_mass = 1.0f;
		float c_gravity = 1.0f;

		switch (character)
		{
			case engine::Character::Glare:
				model_path = "assets/characters/test/character.b3d";

				//model_path = "assets/geometry/sphere.b3d";
				//model_path = "assets/geometry/torus.b3d";
				
				//model_path = "assets/characters/sonic/sonic.b3d";
				//model_path = "assets/objects/turret/turret.b3d";
				//model_path = "assets/geometry/Multi_Torus.b3d";
				//model_path = "assets/geometry/direction_boxes-4.b3d";

				///model_path = "assets/geometry/Multi_Torus_Pose.b3d";
				

				//model_path = "assets/geometry/boxes.b3d";
				//model_path = "assets/geometry/direction_boxes.b3d";
				//model_path = "assets/geometry/direction_boxes-2.b3d";
				//model_path = "assets/geometry/direction_boxes-3.b3d";
				//model_path = "assets/geometry/direction_boxes-move-up.b3d";
				//model_path = "assets/geometry/direction_boxes-move-left.b3d";
				//model_path = "assets/geometry/direction_boxes-move-forward.b3d";
				//model_path = "assets/geometry/direction_boxes-scale.b3d";
				//model_path = "assets/geometry/direction_boxes-rotate-x.b3d";
				//model_path = "assets/geometry/direction_boxes-rotate-y.b3d";
				
				///model_path = "assets/geometry/direction_boxes-rotate-z.b3d";

				//model_path = "assets/geometry/anims_with_full_rotations_between_keys.dae";
				
				//model_path = "assets/geometry/vampire/dancing_vampire.dae";
				//model_path = "assets/geometry/direction_boxes.b3d";
				//model_path = "assets/objects/skeleton_test/a.b3d";

				//model_path = "assets/characters/nanosuit/nanosuit.obj";

				break;
			default:
				//assert(false, "Invalid character specified.");

				break;
		}

		//auto player = load_model(world, "assets/tests/model_test/cube.b3d", parent, EntityType::Player);
		auto player = create_pivot(world, parent, EntityType::Player);
		
		auto player_model = load_model(world, model_path, player);

		registry.emplace<NameComponent>(player, name);

		world.apply_transform(player, tform);

		attach_physics(world, player, MotionFlags::StandardMovement);

		auto& resource_manager = world.get_resource_manager();

		auto collision_data = resource_manager.generate_capsule_collision(1.0f, 2.0f); // get_collision_shape(CollisionShape::Capsule, 1.0f, 2.0f);

		float mass = 1.0f;

		//auto interaction_mask = CollisionGroup::All;
		auto solid_mask = CollisionGroup::Actor;

		attach_collision(world, player, collision_data.collision_shape, EntityType::Player, mass);

		registry.emplace<PlayerState>(player, PlayerState::Action::Default, index);

		//world.set_parent(camera, player);

		return player;
    }
	
	Entity create_player(World& world, math::Vector position, Character character, const std::string& name, Entity parent, PlayerIndex index)
	{
		return create_player(world, { position, { 0.0f, math::radians(180.0f), 0.0f }, { 1.0f, 1.0f, 1.0f } }, character, name, parent, index);
	}

	Character get_character(const std::string& char_name)
	{
		std::string name = util::lowercase(char_name);

		if (name == "glare") // "default"
		{
			return Character::Glare;
		}

		return Character::Default;
	}
}