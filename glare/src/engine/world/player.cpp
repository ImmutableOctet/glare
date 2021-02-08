#include "player.hpp"
#include "world.hpp"
#include "physics.hpp"

#include <util/string.hpp>

#include <engine/resource_manager.hpp>
#include <engine/name_component.hpp>
#include <engine/collision.hpp>

//#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet/BulletCollision/btBulletCollisionCommon.h>

#include <algorithm>
#include <string>

// Debugging related:
#include <iostream>

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

				/*
				model_path
				=
				"assets/characters/nanosuit/nanosuit.obj"
				//"assets/tests/model_test/Sonic/Sonic.b3d"
				;
				*/

				break;
			default:
				ASSERT("Invalid character specified.");

				break;
		}

		auto player = load_model(world, "assets/tests/model_test/cube.b3d", parent, EntityType::Player); //create_pivot(world, parent, EntityType::Player);
		auto player_model = load_model(world, model_path, player);

		{
			auto model_t = world.get_transform(player_model);
			
			std::cout << model_t.get_local_position() << std::endl;
		}

		//auto player = player_model;

		registry.emplace<NameComponent>(player, name);

		auto t = world.get_transform(player);

		t.set_position(position);
		t.set_rotation(rotation);
		t.set_scale(scale);

		attach_physics(world, player, MotionFlags::StandardMovement);

		auto& resource_manager = world.get_resource_manager();

		auto collision_data = resource_manager.get_capsule_collision(1.0f, 2.0f); // get_collision_shape(CollisionShape::Capsule, 1.0f, 2.0f);

		float mass = 1.0f;

		auto interaction_mask = CollisionMask::All;
		auto solid_mask = CollisionMask::StandardObject;

		attach_collision(world, player, collision_data, mass, interaction_mask, solid_mask);

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