#pragma once

#include <math/types.hpp>

#include <optional>
#include <vector>

#include <cstdint>

namespace engine
{
	struct SkeletalKeySequence
	{
		using index_t = std::uint32_t; // int;

		struct KeyVector
		{
			/*
			union
			{
				// Can be accessed as a position or scale.
				math::Vector position;
				math::Vector scale;
				math::Vector value;
			};

			KeyVector() = default;
			KeyVector(const KeyVector& v) = default
			KeyVector(KeyVector&&) noexcept = default;

			inline KeyVector(const math::Vector& vec, float timestamp)
				: value(vec), timestamp(timestamp) {}
			*/

			math::Vector value;

			float timestamp;
		};

		struct KeyRotation
		{
			/*
			union
			{
				math::Quaternion orientation;
				math::Quaternion rotation;
				math::Quaternion value;
			};

			KeyRotation() = default;
			KeyRotation(const KeyRotation&) = default;
			KeyRotation(KeyRotation&&) noexcept = default;

			inline KeyRotation(const math::Quaternion& quat, float timestamp)
				: value(quat), timestamp(timestamp) {}
			*/

			math::Quaternion value;

			float timestamp;
		};

		using KeyPosition = KeyVector;
		using KeyScale    = KeyVector;

		// Retrieves the index of an element of `data` whose `timestamp` value is greater than or equal to `timestamp`.
		template <typename Collection>
		static std::optional<index_t> get_index(const Collection& data, float timestamp)
		{
			if (data.empty())
			{
				return std::nullopt;
			}

			for (std::size_t index = 0; index < (data.size() - 1); ++index)
			{
				if (timestamp < (data[static_cast<std::size_t>(index + 1)].timestamp))
				{
					return static_cast<index_t>(index);
				}
			}

			if (data.size() == 1)
			{
				return 0;
			}

			return std::nullopt;
		}

		// Computes a linear blend factor for `timestamp` using the area between `prev_timestamp` and `next_timestamp`.
		static float linear_blend_factor(float timestamp, float prev_timestamp, float next_timestamp);

		inline std::optional<index_t> get_position_index(float timestamp) const { return get_index(positions, timestamp); }
		inline std::optional<index_t> get_rotation_index(float timestamp) const { return get_index(rotations, timestamp); }
		inline std::optional<index_t> get_scale_index(float timestamp)    const { return get_index(scales, timestamp);    }

		math::Vector interpolated_position(float timestamp) const;
		math::Quaternion interpolated_rotation(float timestamp, bool flip_direction=false) const;
		math::Vector interpolated_scale(float timestamp) const;

		math::Matrix interpolated_position_matrix(float timestamp) const;
		math::Matrix interpolated_rotation_matrix(float timestamp, bool flip_direction=false) const;
		math::Matrix interpolated_scale_matrix(float timestamp) const;

		math::Matrix interpolated_matrix(float timestamp, bool flip_rotation=false) const;

		std::vector<KeyPosition> positions;
		std::vector<KeyRotation> rotations;
		std::vector<KeyScale>    scales;
	};
}