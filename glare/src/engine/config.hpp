#pragma once

#include <math/types.hpp>

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

	// TODO: Refactor to support multiple (local) player definitions.
	struct PlayerConfig
	{
		struct Player
		{
			std::string name;
			std::string character;
		};

		std::string character_path = "assets/characters"; // std::filesystem::path

		Player default_player;
	};

	struct Config
	{
		using Graphics = GraphicsConfig;
		using Players  = PlayerConfig;

		Graphics graphics;
		Players   players;
	};
}