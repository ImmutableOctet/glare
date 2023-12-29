#pragma once

#include "types.hpp"
#include "animation_slice.hpp"
#include "animation_sequence.hpp"

#include <util/small_vector.hpp>

//#include <vector>
#include <unordered_map>
#include <optional>

namespace engine
{
	class AnimationRepository
	{
		public:
			using AnimationLayerIndex = std::size_t;

			inline static constexpr std::size_t animation_layer_bits = (sizeof(AnimationLayerMask) * 8);

			//using SliceContainer = std::vector<std::pair<AnimationID, AnimationSlice>>
			using SliceContainer = std::unordered_map<AnimationID, AnimationSlice>;

			//using SequenceContainer = std::vector<std::pair<AnimationID, AnimationSequence>>
			using SequenceContainer = std::unordered_map<AnimationID, AnimationSequence>;

			// NOTE: Indices correspond to bit masks.
			using LayerContainer = util::small_vector<AnimationLayerID, 4>;

			using BoneLayerMap = std::unordered_map<BoneID, AnimationLayerMask>;

			// Sections of the animation data to be referenced by an `AnimationID`.
			SliceContainer slices;

			// Sequences of slices (or other sequences) to be references by an `AnimationID`.
			SequenceContainer sequences;

			// Set of animation layer identifiers, where each layer represents an index and bitmask.
			LayerContainer layers;

			// Mapping of bone identifiers to their respective animation layer (bitmask).
			BoneLayerMap bone_layers;

			const AnimationSlice* get_slice(AnimationID animation_id) const;
			const AnimationSequence* get_sequence(AnimationID animation_id) const;

			bool has_layer(AnimationLayerID layer_id) const;

			std::optional<AnimationLayerMask> get_layer_mask(AnimationLayerID layer_id) const;
			std::optional<AnimationLayerMask> get_layer_mask_from_index(AnimationLayerIndex layer_index) const;

			std::optional<AnimationLayerIndex> get_layer_index(AnimationLayerID layer_id) const;

			std::optional<AnimationLayerIndex> add_layer(AnimationLayerID layer_id);

			// Returns the furthest (i.e. largest) ending frame index in `slices`.
			// 
			// If no data could be found, this will return zero, implying that
			// the first frame is the earliest starting frame that can be used.
			FrameIndex get_furthest_frame_sliced() const;
	};
}