#include "skeletal_key_sequence.hpp"

#include <math/math.hpp>

#include <type_traits>

namespace engine
{
	template <typename KeyType, typename MixFn>
	std::decay_t<KeyType> interpolate(const KeyType& from_frame, const KeyType& to_frame, float blend_factor, MixFn&& mix_fn)
	{
		return mix_fn(from_frame.value, to_frame.value, blend);
	}

	template <typename Keys, typename index_t, typename MixFn>
	static auto interpolate(float timestamp, const Keys& keys, index_t index, MixFn&& mix_fn, bool flip_direction=false) -> decltype(Keys::value_type::value)
	{
		assert(!keys.empty());

		if (keys.size() == 1)
		{
			return keys[0].value;
		}

		const auto& prev = keys[(index)];
		const auto& next = keys[(index + 1)];

		const auto blend = (flip_direction)
			? SkeletalKeySequence::blend_factor(timestamp, next.timestamp, prev.timestamp)
			: SkeletalKeySequence::blend_factor(timestamp, prev.timestamp, next.timestamp)
		;

		return interpolate();
	}

	float SkeletalKeySequence::linear_blend_factor(float timestamp, float prev_timestamp, float next_timestamp)
	{
		const auto elapsed_time = (timestamp - prev_timestamp);
		const auto remaining_time = (next_timestamp - prev_timestamp);

		return (elapsed_time / remaining_time);
	}

	math::Vector SkeletalKeySequence::interpolated_position(float timestamp) const
	{
		return interpolate
		(
			timestamp, positions,
			get_position_index(timestamp),

			[](const math::Vector& prev, const math::Vector& next, float blend)
			{
				return glm::mix(prev, next, blend);
			}
		);
	}

	math::Quaternion SkeletalKeySequence::interpolated_rotation(float timestamp, bool flip_direction) const
	{
		return interpolate
		(
			timestamp, rotations,
			get_rotation_index(timestamp),
			
			[](const math::Quaternion& prev, const math::Quaternion& next, float blend)
			{
				return math::slerp(prev, next, blend); // glm::slerp(...);
			},

			flip_direction
		);
	}

	math::Vector SkeletalKeySequence::interpolated_scale(float timestamp) const
	{
		return interpolate
		(
			timestamp, scales,
			get_scale_index(timestamp),
			
			[](const math::Vector& prev, const math::Vector& next, float blend)
			{
				return glm::mix(prev, next, blend);
			}
		);
	}

	math::Matrix SkeletalKeySequence::interpolated_position_matrix(float timestamp) const
	{
		return glm::translate(glm::mat4(1.0f), interpolated_position(timestamp));
	}

	math::Matrix SkeletalKeySequence::interpolated_rotation_matrix(float timestamp, bool flip_direction) const
	{
		return glm::toMat4(interpolated_rotation(timestamp, flip_direction));
	}

	math::Matrix SkeletalKeySequence::interpolated_scale_matrix(float timestamp) const
	{
		return glm::scale(glm::mat4(1.0f), interpolated_scale(timestamp));
	}

	math::Matrix SkeletalKeySequence::interpolated_matrix(float timestamp, bool flip_rotation) const
	{
		const auto translation = interpolated_position_matrix(timestamp);
		const auto rotation    = interpolated_rotation_matrix(timestamp, flip_rotation);
		const auto scale       = interpolated_scale_matrix(timestamp);

		return (translation * rotation * scale);
	}
}