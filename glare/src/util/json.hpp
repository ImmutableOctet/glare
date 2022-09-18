#pragma once

#include <types.hpp>

#include <math/types.hpp>
//#include <graphics/types.hpp>

#include <nlohmann/json.hpp>

//#include <tuple>
//#include <string>
//#include <string_view>

namespace util
{
	using json = nlohmann::json;

	math::TransformVectors get_transform(const json& data);

	math::Vector3D to_vector(const json& j);
	math::vec2i to_vec2i(const json& j);

	graphics::ColorRGBA to_color(const json& j, float default_alpha=1.0f);
	graphics::ColorRGB to_color_rgb(const json& j);

	template <typename T, typename UIDType>
	inline T get_value(const json& data, const UIDType& name, T default_value={})
	{
		return (data.contains(name) ? data[name].get<T>() : default_value);
	}

	template <typename T, typename UIDType, typename get_fn>
	inline void retrieve_value(const json& data, const UIDType& name, T& dest, get_fn&& fn)
	{
		if (data.contains(name))
		{
			dest = fn(data, name);
		}
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

	template <typename T, typename UIDType>
	inline void retrieve_value(const json& data, const UIDType& name, T& dest)
	{
		retrieve_value
		(
			data, name, dest,
			[](const json& data, const UIDType& name)
			{
				return data[name].get<T>();
			}
		);
	}

	template <typename UIDType>
	inline void retrieve_value(const json& data, const UIDType& name, math::Vector& dest)
	{
		retrieve_value
		(
			data, name, dest,
			[](const json& data, const UIDType& name)
			{
				return to_vector(data[name]);
			}
		);
	}

	template <typename UIDType>
	inline void retrieve_value(const json& data, const UIDType& name, math::vec2i& dest)
	{
		retrieve_value
		(
			data, name, dest,
			[](const json& data, const UIDType& name)
			{
				return to_vec2i(data[name]);
			}
		);
	}

	template <typename UIDType>
	inline void retrieve_value(const json& data, const UIDType& name, graphics::ColorRGBA& dest, float default_alpha=1.0f)
	{
		retrieve_value
		(
			data, name, dest,
			[](const json& data, const UIDType& name)
			{
				return to_color(data[name], default_alpha);
			}
		);
	}

	template <typename UIDType>
	inline void retrieve_value(const json& data, const UIDType& name, math::TransformVectors& dest)
	{
		retrieve_value
		(
			data, name, dest,
			[](const json& data, const UIDType& name)
			{
				return get_transform(data[name]);
			}
		);
	}
}