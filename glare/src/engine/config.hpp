#pragma once

#include <math/types.hpp>

#include <string>

namespace engine
{
	struct GraphicsConfig
	{
		using Resolution = math::vec2i;

		struct Shadows
		{
			Resolution resolution = { 8192, 8192 };
			Resolution cubemap_resolution = { 2048, 2048 };

			bool enabled = true;
		};

		struct Parallax
		{
			float min_layers = 32; // height_map_min_layers;
			float max_layers = 64; // height_map_max_layers;
		};

		Shadows shadows;
		Parallax parallax;
	};

	struct ObjectConfig
	{
		std::string object_path; // std::filesystem::path
	};

	// TODO: Refactor to support multiple (local) player definitions.
	struct PlayerConfig
	{
		struct Player
		{
			std::string name;
			std::string character;
		};

		std::string character_path = "characters"; // std::filesystem::path

		Player default_player;
	};

	struct EntityConfig
	{
		std::string archetype_path = "engine/archetypes"; // std::filesystem::path
	};

	struct Config
	{
		using Graphics = GraphicsConfig;
		using Objects  = ObjectConfig;
		using Players  = PlayerConfig;
		using Entities = EntityConfig;

		Graphics graphics;
		Objects  objects;
		Players  players;
		Entities entities;
	};
}