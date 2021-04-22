#pragma once

#include <types.hpp>
#include <math/math.hpp>
//#include <graphics/types.hpp>

#include <nlohmann/json.hpp>

#include <tuple>
//#include <string>
//#include <string_view>

namespace util
{
	using json = nlohmann::json;

	inline math::Vector3D to_vector(const json& j)
	{
		return { j["x"].get<float>(), j["y"].get<float>(), j["z"].get<float>() };
	}

	inline math::vec2i to_vec2i(const json& j)
	{
		return { j["x"].get<int>(), j["y"].get<int>() };
	}

	inline graphics::ColorRGBA to_color(const json& j, float default_alpha=1.0f)
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

	inline graphics::ColorRGB to_color_rgb(const json& j)
	{
		return
		{
			j["r"].get<float>(),
			j["g"].get<float>(),
			j["b"].get<float>()
		};
	}

	template <typename T, typename UIDType>
	inline T get_value(const json& data, const UIDType& name, T default_value={})
	{
		return (data.contains(name) ? data[name].get<T>() : default_value);
	}

	template <typename UIDType>
	inline math::Vector get_vector(const json& data, const UIDType& vector_name, math::Vector default_value={0.0f, 0.0f, 0.0f})
	{
		return (data.contains(vector_name) ? to_vector(data[vector_name]) : default_value);
	}

	template <typename UIDType>
	inline math::vec2i get_vec2i(const json& data, const UIDType& vector_name, math::vec2i default_value = { 0, 0 })
	{
		return (data.contains(vector_name) ? to_vec2i(data[vector_name]) : default_value);
	}

	template <typename UIDType>
	inline graphics::ColorRGBA get_color(const json& data, const UIDType& color_name, graphics::ColorRGBA default_value={1.0f, 1.0f, 1.0f, 1.0f}, float default_alpha=1.0f)
	{
		return (data.contains(color_name) ? to_color(data[color_name], default_alpha) : default_value);
	}

	template <typename UIDType>
	inline graphics::ColorRGB get_color_rgb(const json& data, const UIDType& color_name, graphics::ColorRGB default_value = { 1.0f, 1.0f, 1.0f})
	{
		return (data.contains(color_name) ? to_color_rgb(data[color_name]) : default_value);
	}

	inline math::TransformVectors get_transform(const json& data)
	{
		auto position = get_vector(data, "position");
		auto rotation = math::radians(get_vector(data, "rotation"));
		auto scale = get_vector(data, "scale", { 1.0f, 1.0f, 1.0f });

		return { position, rotation, scale };
	}
}