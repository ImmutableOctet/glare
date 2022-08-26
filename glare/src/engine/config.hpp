#pragma once

#include <math/math.hpp>
#include <util/json.hpp>

namespace engine
{
	class Config
	{
		public:
			using Resolution = math::vec2i;
			using JSON = util::json;
			
			struct
			{
				struct
				{
					Resolution resolution = { 8192, 8192 };
					Resolution cubemap_resolution = { 2048, 2048 };

					bool enabled = true;
				} shadow;

				struct
				{
					float min_layers = 32; // height_map_min_layers;
					float max_layers = 64; // height_map_max_layers;
				} parallax;
			} graphics;

			Config() = default;
			Config(const Config&) = default;
			Config(Config&&) = default;
	};

	void from_json(const Config::JSON& data, Config& cfg);
}