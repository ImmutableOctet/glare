#pragma once

#include "types.hpp"
//#include "skeleton.hpp"

#include "skeletal_key_sequence.hpp"

#include <math/types.hpp>

//#include <util/small_vector.hpp>
#include <vector>

namespace engine
{
	struct SkeletalFrameData
	{
		using KeyVector   = SkeletalKeySequence::KeyVector;

		using KeyPosition = SkeletalKeySequence::KeyPosition;
		using KeyRotation = SkeletalKeySequence::KeyRotation;
		using KeyScale    = SkeletalKeySequence::KeyScale;

		// A vector of skeletal key sequences, representing each bone by its index.
		using SkeletalKeyFrames = std::vector<SkeletalKeySequence>;

		inline const SkeletalKeySequence* get_sequence(BoneIndex bone_index) const
		{
			if (bone_index >= size())
			{
				return {};
			}

			return &(skeletal_sequences[bone_index]);
		}

		inline std::size_t size() const
		{
			return skeletal_sequences.size();
		}

		inline bool empty() const
		{
			return skeletal_sequences.empty();
		}

		inline bool exists() const
		{
			return (!empty());
		}

		inline explicit operator bool() const
		{
			return exists();
		}

		SkeletalKeyFrames skeletal_sequences;

		float duration = 0.0f;
	};
}