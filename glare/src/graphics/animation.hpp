#pragma once

#include <vector>

#include <math/math.hpp>

#include "types.hpp"
//#include "skeleton.hpp"

struct aiNodeAnim;
struct aiVectorKey;
struct aiQuatKey;

namespace graphics
{
	struct Animation
	{
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
			KeyVector(KeyVector&&) = default;

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
			KeyRotation(KeyRotation&&) = default;

			inline KeyRotation(const math::Quaternion& quat, float timestamp)
				: value(quat), timestamp(timestamp) {}
			*/

			math::Quaternion value;
			float timestamp;
		};

		using KeyPosition = KeyVector;
		using KeyScale    = KeyVector;

		struct KeySequence
		{
			using index_t = int;

			static float blend_factor(float timestamp, float prev_timestamp, float next_timestamp);

			static std::vector<KeyVector>& load_vectors(const aiVectorKey* keys, unsigned int key_count, std::vector<KeyVector>& out, const math::Matrix& orientation);
			static std::vector<KeyRotation>& load_rotations(const aiQuatKey* keys, unsigned int key_count, std::vector<KeyRotation>& out, const math::Matrix& orientation);

			KeySequence& load(const aiNodeAnim& channel_data, const math::Matrix orientation);

			KeySequence() = default;

			inline KeySequence(const aiNodeAnim& channel_data, const math::Matrix orientation)
				: KeySequence() { load(channel_data, orientation); }

			template <typename Collection>
			index_t get_index(float timestamp, const Collection& data) const
			{
				for (index_t index = 0; index < (data.size()-1); ++index)
				{
					if (timestamp < (data[index+1].timestamp))
					{
						return index;
					}
				}

				if (data.size() == 1)
				{
					return 0;
				}

				// Invalid timestamp specified.
				ASSERT(false);
			}

			inline auto get_position_index(float timestamp) const { return get_index(timestamp, positions); }
			inline auto get_rotation_index(float timestamp) const { return get_index(timestamp, rotations); }
			inline auto get_scale_index(float timestamp)    const { return get_index(timestamp, scales);    }

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

		using FrameData = std::unordered_map<BoneID, KeySequence>;

		float duration = 0.0f;
		float rate = 0.0f;

		FrameData skeletal_sequence;

		const KeySequence* get_sequence(BoneID bone_id) const;
	};
}