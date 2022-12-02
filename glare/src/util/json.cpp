#pragma once

#include "json.hpp"

#include <math/conversion.hpp>

#include <fstream>

// Debugging related:
#include "log.hpp"

namespace util
{
	json load_json(const std::filesystem::path& path)
	{
		try
		{
			std::ifstream map_data_stream(path);

			return util::json::parse(map_data_stream, nullptr, true, true);
		}
		catch (const std::exception& e)
		{
			print("JSON Error: {}", e.what());

			throw e;
		}
	}

	math::TransformVectors get_transform(const json& data)
	{
		auto position = get_vector(data, "position");
		auto rotation = math::radians(get_vector(data, "rotation"));
		auto scale = get_vector(data, "scale", { 1.0f, 1.0f, 1.0f });

		return { position, rotation, scale };
	}

	math::Vector3D to_vector(const json& j)
	{
		return { j["x"].get<float>(), j["y"].get<float>(), j["z"].get<float>() };
	}

	math::vec2i to_vec2i(const json& j)
	{
		return { j["x"].get<int>(), j["y"].get<int>() };
	}

	graphics::ColorRGBA to_color(const json& j, float default_alpha)
	{
		return
		{
			j["r"].get<float>(),
			j["g"].get<float>(),
			j["b"].get<float>(),
			
			(
				(j.contains("a"))
				? j["a"].get<float>()
				: default_alpha
			)
		};
	}

	graphics::ColorRGB to_color_rgb(const json& j)
	{
		return
		{
			j["r"].get<float>(),
			j["g"].get<float>(),
			j["b"].get<float>()
		};
	}
}