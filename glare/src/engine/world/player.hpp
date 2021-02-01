#pragma once

#include <engine/types.hpp>
#include <string>

namespace engine
{
	//class ResourceManager;

	using PlayerIndex = std::uint16_t;

	enum class Character : std::uint8_t
	{
		Glare = 0,

		Default = Glare
	};

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

	//constexpr std::string
	extern const char* DEFAULT_PLAYER_NAME;

	Entity create_player(World& world, const math::TransformVectors& tform, Character character=Character::Default, const std::string& name=DEFAULT_PLAYER_NAME, Entity parent=null, PlayerIndex index=PlayerState::NoPlayer);
	Entity create_player(World& world, math::Vector position = {}, Character character=Character::Default, const std::string& name=DEFAULT_PLAYER_NAME, Entity parent=null, PlayerIndex index=PlayerState::NoPlayer);

	Character get_character(const std::string& char_name);
}