#include "skeletal_key_sequence.hpp"

#include <math/math.hpp>

#include <type_traits>

namespace engine
{
	float SkeletalKeySequence::linear_blend_factor(float timestamp, float prev_timestamp, float next_timestamp)
	{
		const auto elapsed_time = (timestamp - prev_timestamp);
		const auto remaining_time = (next_timestamp - prev_timestamp);

		return (elapsed_time / remaining_time);
	}

	math::Vector SkeletalKeySequence::interpolated_position(float timestamp, float min_timestamp, float max_timestamp) const
	{
		return interpolate
		(
			timestamp,
			positions,
			get_position_indices(timestamp, min_timestamp, max_timestamp),

			[](const math::Vector& prev, const math::Vector& next, float blend)
			{
				return glm::mix(prev, next, blend);
			}
		);
	}

	math::Quaternion SkeletalKeySequence::interpolated_rotation(float timestamp, float min_timestamp, float max_timestamp, bool flip_direction) const
	{
		return interpolate
		(
			timestamp,
			rotations,
			get_rotation_indices(timestamp, min_timestamp, max_timestamp),
			
			[](const math::Quaternion& prev, const math::Quaternion& next, float blend)
			{
				return math::slerp(prev, next, blend); // glm::slerp(...);
			},

			flip_direction
		);
	}

	math::Vector SkeletalKeySequence::interpolated_scale(float timestamp, float min_timestamp, float max_timestamp) const
	{
		return interpolate
		(
			timestamp,
			scales,
			get_scale_indices(timestamp, min_timestamp, max_timestamp),
			
			[](const math::Vector& prev, const math::Vector& next, float blend)
			{
				return glm::mix(prev, next, blend);
			}
		);
	}

	math::Matrix SkeletalKeySequence::interpolated_position_matrix(float timestamp, float min_timestamp, float max_timestamp) const
	{
		return glm::translate(glm::mat4(1.0f), interpolated_position(timestamp, min_timestamp, max_timestamp));
	}

	math::Matrix SkeletalKeySequence::interpolated_rotation_matrix(float timestamp, float min_timestamp, float max_timestamp, bool flip_direction) const
	{
		return glm::toMat4(interpolated_rotation(timestamp, min_timestamp, max_timestamp, flip_direction));
	}

	math::Matrix SkeletalKeySequence::interpolated_scale_matrix(float timestamp, float min_timestamp, float max_timestamp) const
	{
		return glm::scale(glm::mat4(1.0f), interpolated_scale(timestamp, min_timestamp, max_timestamp));
	}

	math::Matrix SkeletalKeySequence::interpolated_matrix(float timestamp, float min_timestamp, float max_timestamp, bool flip_rotation) const
	{
		const auto translation = interpolated_position_matrix(timestamp, min_timestamp, max_timestamp);
		const auto rotation    = interpolated_rotation_matrix(timestamp, min_timestamp, max_timestamp, flip_rotation);
		const auto scale       = interpolated_scale_matrix(timestamp, min_timestamp, max_timestamp);

		return (translation * rotation * scale);
	}
}