#include "player.hpp"

#include <engine/input/types.hpp>
#include <engine/input/components/input_component.hpp>

#include <engine/world/world.hpp>
#include <engine/world/entity.hpp>
#include <engine/world/graphics_entity.hpp>

#include <engine/world/motion/components/motion_component.hpp>
#include <engine/world/motion/components/alignment_component.hpp>

#include <engine/world/physics/collision.hpp>

#include <util/string.hpp>
#include <util/log.hpp>

#include <engine/resource_manager/resource_manager.hpp>

#include <engine/components/name_component.hpp>
#include <engine/components/model_component.hpp>

//#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet/btBulletCollisionCommon.h>

#include <algorithm>
#include <fstream>
#include <utility>

namespace engine
{
	const char* DEFAULT_PLAYER_NAME = "Player";
	const char* DEFAULT_CHARACTER = "Glare";
	const char* DEFAULT_CHARACTER_ROOT_PATH = "assets/characters";

	CharacterData load_character_data
	(
		const std::string& character,
		const filesystem::path& root_path
	)
	{
		util::json character_data;

		std::filesystem::path character_path = root_path;

		character_path /= character;

		std::string character_name;

		print("Loading player JSON...");

		try
		{
			std::ifstream player_data_stream((character_path / "instance.json"));

			character_data = util::json::parse(player_data_stream);

			character_name = util::get_value<std::string>(character_data, "name", character);
		}
		catch (std::exception& e)
		{
			print_warn("Error parsing JSON file: {}", e.what());

			assert(false);
		}

		return { std::move(character_name), std::move(character_path), std::move(character_data) };
	}

	Entity load_player_model(World& world, Entity player, const util::json& model_data, const std::filesystem::path& base_path)
	{
		auto& registry = world.get_registry();

		auto model_path = (base_path / util::get_value<std::string>(model_data, "path"));

		auto player_model = load_model(world, model_path.string(), player);

		if (model_data.contains("offset"))
		{
			auto tform = world.get_transform(player_model);

			tform.set_local_position(util::get_vector(model_data, "offset"));
		}
		
		registry.emplace_or_replace<NameComponent>(player_model, "player_model");

		return player_model;
	}

    Entity create_player
	(
		World& world,
		const CharacterData& character_data,
		const std::string& name,
		Entity parent,
		PlayerIndex index
	)
    {
		const auto& character_name = std::get<0>(character_data);
		const auto& character_path = std::get<1>(character_data);
		const auto& character      = std::get<2>(character_data);

		auto& registry = world.get_registry();

		auto player = create_pivot(world, parent, EntityType::Player);

		world.set_name(player, ((name.empty()) ? character_name : name));

		registry.emplace<PlayerState>(player, PlayerState::Action::Default, index);
		
		auto& motion = registry.emplace<MotionComponent>(player);

		//motion.apply_gravity = false;

		auto player_model = load_player_model(world, player, character["model"], character_path);

		// Set the alignment entity to `player_model`, since it's meant to be floor-aligned.
		registry.emplace<AlignmentComponent>(player, player_model);

		//generate_player_collision(world, player, character["collision"]);

		registry.emplace<InputComponent>(player, static_cast<InputStateIndex>(index));
		//registry.emplace<PlayerControlComponent>(player);

		//world.set_parent(camera, player);

		return player;
    }
}