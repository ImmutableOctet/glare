#include "config.hpp"

namespace engine
{
	void from_json(const Config::JSON& data, Config& cfg)
	{
		// Graphics:
		const auto& graphics = data["graphics"];
		auto& g = cfg.graphics;

		const auto& parallax = graphics["parallax"];

		util::retrieve_value(parallax, "min_layers", g.parallax.min_layers);
		util::retrieve_value(parallax, "max_layers", g.parallax.max_layers);

		const auto& shadow = graphics["shadow"];

		util::retrieve_value(shadow, "resolution", g.shadow.resolution);
		util::retrieve_value(shadow, "cubemap_resolution", g.shadow.cubemap_resolution);
	}
}