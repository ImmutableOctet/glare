#pragma once

#include <types.hpp>
#include <math/math.hpp>

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

	template <typename T, typename UIDType>
	inline T get_value(const json& data, const UIDType& name, T default_value={})
	{
		return (data.contains(name) ? data[name].get<T>() : default_value);
	}

	template <typename UIDType>
	inline math::Vector get_vector(const json& data, const UIDType& vector_name, math::Vector default_value={ 0.0f, 0.0f, 0.0f })
	{
		return (data.contains(vector_name) ? util::to_vector(data[vector_name]) : default_value);
	}

	inline math::TransformVectors get_transform(const json& data)
	{
		auto position = get_vector(data, "position");
		auto rotation = math::radians(get_vector(data, "rotation"));
		auto scale = get_vector(data, "scale", { 1.0f, 1.0f, 1.0f });

		return { position, rotation, scale };
	}
}