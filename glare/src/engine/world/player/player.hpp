#pragma once

#include <engine/world/types.hpp>

#include <util/json.hpp>

#include <filesystem>
#include <string>
#include <tuple>

namespace engine
{
	//class ResourceManager;

	struct CollisionData;

	// TODO: Remove. (replaced by `PlayerComponent` and co.)
	struct PlayerState
	{
		using Index = PlayerIndex;

		static constexpr Index NoPlayer = 0;

		enum class Action
		{
			Common,
			Walk,
			Run,
			Jump,
			Jumpdash,
			Dive,
			Grab,
			Attack,
			Wall,
			WallJump,
			Ledge,
			Hang,

			Default = Common,
		};

		Action action = Action::Default;
		Index index = NoPlayer;
	};

	using CharacterData = std::tuple
	<
		std::string,           // Name
		std::filesystem::path, // Base path
		util::json             // Character data
	>;

	//constexpr std::string
	extern const char* DEFAULT_PLAYER_NAME;
	extern const char* DEFAULT_CHARACTER;
	extern const char* DEFAULT_CHARACTER_ROOT_PATH;

	CharacterData load_character_data
	(
		const std::string& character,
		const std::filesystem::path& root_path=DEFAULT_CHARACTER_ROOT_PATH
	);

	Entity load_player_model
	(
		World& world,
		Entity player,
		const util::json& model_data,
		const std::filesystem::path& base_path
	);

	void generate_player_collision(World& world, Entity player, const util::json& collision_data);

	Entity create_player
	(
		World& world,
		const CharacterData& character_data,
		const std::string& name=DEFAULT_PLAYER_NAME,
		Entity parent=null,
		PlayerIndex index=PlayerState::NoPlayer
	);
}