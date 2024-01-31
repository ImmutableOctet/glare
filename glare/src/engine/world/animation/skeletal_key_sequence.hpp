#pragma once

#include "types.hpp"

#include <math/types.hpp>

#include <optional>
#include <utility>
#include <vector>

#include <cstdint>
#include <cassert>

namespace engine
{
	struct SkeletalKeySequence
	{
		using index_t = std::uint32_t;

		template <typename T, typename TimestampType=float>
		struct KeyData
		{
			using value_type = T;
			using timestamp_type = TimestampType;

			value_type value;

			timestamp_type timestamp;

			explicit operator value_type() const
			{
				return value;
			}

			explicit operator const value_type&() const
			{
				return value;
			}
		};

		using KeyVector   = KeyData<math::Vector>;
		using KeyRotation = KeyData<math::Quaternion>;

		using KeyPosition = KeyVector;
		using KeyScale    = KeyVector;

		// Retrieves the index of an element of `data` whose `timestamp` value is greater than or equal to `timestamp`,
		// and within the range described by `min_timestamp` and `max_timestamp`.
		template <typename Collection>
		static std::pair<std::optional<index_t>, std::optional<index_t>> get_indices(const Collection& data, float timestamp, float min_timestamp, float max_timestamp)
		{
			if (!data.empty())
			{
				for (index_t index = {}; index < static_cast<index_t>(data.size() - 1); ++index)
				{
					const auto& entry = data[index];

					if ((entry.timestamp >= min_timestamp) && (entry.timestamp <= max_timestamp))
					{
						if (entry.timestamp >= timestamp)
						{
							const auto next_index = (index + 1);
							const auto& next_entry = data[next_index];

							if ((next_entry.timestamp >= min_timestamp) && (next_entry.timestamp <= max_timestamp))
							{
								return { index, next_index };
							}

							return { index, std::nullopt };
						}
					}
				}
			}

			return { std::nullopt, std::nullopt };
		}

		template <typename KeyType, typename MixFn>
		static auto interpolate(const KeyType& from_frame, const KeyType& to_frame, float blend_factor, MixFn&& mix_fn)
		{
			return mix_fn(from_frame.value, to_frame.value, blend_factor);
		}

		template <typename Keys, typename index_t, typename MixFn>
		static auto interpolate
		(
			float timestamp,
			
			const Keys& keys,
			
			std::pair<std::optional<index_t>, std::optional<index_t>> indices,

			MixFn&& mix_fn,
			
			bool flip_direction=false
		) -> decltype(Keys::value_type::value)
		{
			const auto& [from_index, to_index] = indices;

			assert(from_index);
			assert(static_cast<std::size_t>(*from_index) < keys.size());

			const auto& prev = keys[*from_index];

			if (!to_index)
			{
				return prev.value;
			}

			const auto& next = keys[*to_index];

			const auto blend_factor = (flip_direction)
				? SkeletalKeySequence::linear_blend_factor(timestamp, next.timestamp, prev.timestamp)
				: SkeletalKeySequence::linear_blend_factor(timestamp, prev.timestamp, next.timestamp)
			;

			return interpolate(prev, next, blend_factor, std::forward<MixFn>(mix_fn));
		}

		// Computes a linear blend factor for `timestamp` using the area between `prev_timestamp` and `next_timestamp`.
		static float linear_blend_factor(float timestamp, float prev_timestamp, float next_timestamp);

		inline auto get_position_indices(float timestamp, float min_timestamp, float max_timestamp) const { return get_indices(positions, timestamp, min_timestamp, max_timestamp); }
		inline auto get_rotation_indices(float timestamp, float min_timestamp, float max_timestamp) const { return get_indices(rotations, timestamp, min_timestamp, max_timestamp); }
		inline auto get_scale_indices(float timestamp, float min_timestamp, float max_timestamp)    const { return get_indices(scales, timestamp, min_timestamp, max_timestamp);    }

		math::Vector interpolated_position(float timestamp, float min_timestamp, float max_timestamp) const;
		math::Quaternion interpolated_rotation(float timestamp, float min_timestamp, float max_timestamp, bool flip_direction=false) const;
		math::Vector interpolated_scale(float timestamp, float min_timestamp, float max_timestamp) const;

		math::Matrix interpolated_position_matrix(float timestamp, float min_timestamp, float max_timestamp) const;
		math::Matrix interpolated_rotation_matrix(float timestamp, float min_timestamp, float max_timestamp, bool flip_direction=false) const;
		math::Matrix interpolated_scale_matrix(float timestamp, float min_timestamp, float max_timestamp) const;

		math::Matrix interpolated_matrix(float timestamp, float min_timestamp, float max_timestamp, bool flip_rotation=false) const;

		std::vector<KeyPosition> positions;
		std::vector<KeyRotation> rotations;
		std::vector<KeyScale>    scales;
	};
}