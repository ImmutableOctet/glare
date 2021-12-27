#include "animation.hpp"

#include <math/math.hpp>
#include <assimp/anim.h>
#include "..\engine\world\animation.hpp"

//struct aiVector3D;
//struct aiQuaternion;

namespace graphics
{
	template <typename KeyTypeIn, typename SizeType=unsigned int, typename KeyTypeOut, typename ConversionFn>
	static std::vector<KeyTypeOut>& load_keys(const KeyTypeIn* keys, SizeType key_count, std::vector<KeyTypeOut>& out, ConversionFn&& cnv_fn)
	{
		for (SizeType i = 0; i < key_count; i++)
		{
			const auto& key_in = keys[i];

			out.push_back(KeyTypeOut { cnv_fn(key_in.mValue), static_cast<float>(key_in.mTime) });
			//out.emplace_back(cnv_fn(key_in.mValue), static_cast<float>(key_in.mTime));
		}

		return out;
	}

	template <typename Keys, typename index_t, typename MixFn>
	static auto interpolate(float timestamp, const Keys& keys, index_t index, MixFn&& mix_fn) -> decltype(Keys::value_type::value)
	{
		ASSERT(!keys.empty());

		if (keys.size() == 1)
		{
			return keys[0].value;
		}

		const auto& prev = keys[(index)];
		const auto& next = keys[(index + 1)];

		float blend = Animation::KeySequence::blend_factor(timestamp, prev.timestamp, next.timestamp);

		return mix_fn(prev.value, next.value, blend);
	}

	float Animation::KeySequence::blend_factor(float timestamp, float prev_timestamp, float next_timestamp)
	{
		float decay = (timestamp - prev_timestamp);
		float transition_length = (next_timestamp - prev_timestamp);

		return (decay / transition_length);
	}

	std::vector<Animation::KeyVector>& Animation::KeySequence::load_vectors(const aiVectorKey* keys, unsigned int key_count, std::vector<KeyVector>& out, const math::Matrix& orientation)
	{
		return load_keys
		(
			keys, key_count, out,
			[&orientation](const aiVector3D& v)
			{
				auto vec = math::to_vector(v);

				return math::Vector3D(orientation * math::Vector4D(vec, 1.0f));
			}
		);
	}

	std::vector<Animation::KeyRotation>& Animation::KeySequence::load_rotations(const aiQuatKey* keys, unsigned int key_count, std::vector<KeyRotation>& out, const math::Matrix& orientation)
	{
		return load_keys
		(
			keys, key_count, out,
			[&orientation](const aiQuaternion& q)
			{
				auto quat = math::to_quat(q);
				return quat; // orientation * ...;
			}
		);
	}

	Animation::KeySequence& Animation::KeySequence::load(const aiNodeAnim& channel_data, const math::Matrix orientation)
	{
		load_vectors(channel_data.mPositionKeys, channel_data.mNumPositionKeys, positions, orientation);
		load_rotations(channel_data.mRotationKeys, channel_data.mNumRotationKeys, rotations, orientation);
		load_vectors(channel_data.mScalingKeys, channel_data.mNumScalingKeys, scales, orientation);

		return *this;
	}

	math::Vector Animation::KeySequence::interpolated_position(float timestamp) const
	{
		return interpolate
		(
			timestamp, positions,
			get_position_index(timestamp),
			[](const math::Vector& prev, const math::Vector& next, float blend)
			{ return glm::mix(prev, next, blend); }
		);
	}

	math::Quaternion Animation::KeySequence::interpolated_rotation(float timestamp) const
	{
		return interpolate
		(
			timestamp, rotations,
			get_rotation_index(timestamp),
			[](const math::Quaternion& prev, const math::Quaternion& next, float blend)
			{ return glm::slerp(prev, next, blend); }
		);
	}

	math::Vector Animation::KeySequence::interpolated_scale(float timestamp) const
	{
		return interpolate
		(
			timestamp, scales,
			get_scale_index(timestamp),
			[](const math::Vector& prev, const math::Vector& next, float blend)
			{ return glm::mix(prev, next, blend); }
		);
	}

	math::Matrix Animation::KeySequence::interpolated_position_matrix(float timestamp) const
	{
		return glm::translate(glm::mat4(1.0f), interpolated_position(timestamp));
	}

	math::Matrix Animation::KeySequence::interpolated_rotation_matrix(float timestamp) const
	{
		return glm::toMat4(interpolated_rotation(timestamp));
	}

	math::Matrix Animation::KeySequence::interpolated_scale_matrix(float timestamp) const
	{
		return glm::scale(glm::mat4(1.0f), interpolated_scale(timestamp));
	}

	math::Matrix Animation::KeySequence::interpolated_matrix(float timestamp) const
	{
		auto translation = interpolated_position_matrix(timestamp);
		auto rotation = interpolated_rotation_matrix(timestamp);
		auto scale = interpolated_scale_matrix(timestamp);

		return (translation * rotation * scale);
	}

	const Animation::KeySequence* Animation::get_sequence(BoneID bone_id) const
	{
		auto it = skeletal_sequence.find(bone_id);

		if (it != skeletal_sequence.end())
		{
			return &(it->second);
		}

		return nullptr;
	}
}