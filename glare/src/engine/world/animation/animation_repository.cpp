#include "animation_repository.hpp"

#include <algorithm>
#include <iterator>
#include <cassert>

namespace engine
{
	const AnimationSlice* AnimationRepository::get_slice(AnimationID animation_id) const
	{
		if (const auto requested_slice = slices.find(animation_id); requested_slice != slices.end())
		{
			return &requested_slice->second;
		}

		return {};
	}

	const AnimationSequence* AnimationRepository::get_sequence(AnimationID animation_id) const
	{
		if (const auto requested_sequence = sequences.find(animation_id); requested_sequence != sequences.end())
		{
			return &requested_sequence->second;
		}

		return {};
	}

	FrameIndex AnimationRepository::get_furthest_frame_sliced() const
	{
		auto furthest_frame = FrameIndex {};

		for (const auto& slice_entry : slices)
		{
			const auto& slice_data = slice_entry.second;

			furthest_frame = std::max(furthest_frame, slice_data.to);
		}

		return furthest_frame;
	}

	bool AnimationRepository::has_layer(AnimationLayerID layer_id) const
	{
		return (std::find(layers.begin(), layers.end(), layer_id) != layers.end());
	}

	std::optional<AnimationLayerMask> AnimationRepository::get_layer_mask(AnimationLayerID layer_id) const
	{
		if (auto layer_index = get_layer_index(layer_id))
		{
			return get_layer_mask_from_index(*layer_index);
		}

		return std::nullopt;
	}

	std::optional<AnimationLayerMask> AnimationRepository::get_layer_mask_from_index(AnimationLayerIndex layer_index) const
	{
		if (layer_index < layers.size())
		{
			assert(layer_index < animation_layer_bits);

			return static_cast<AnimationLayerMask>(static_cast<AnimationLayerMask>(1) << static_cast<AnimationLayerMask>(layer_index));
		}

		return std::nullopt;
	}

	std::optional<AnimationRepository::AnimationLayerIndex> AnimationRepository::get_layer_index(AnimationLayerID layer_id) const
	{
		if (layer_id)
		{
			if (const auto layer_it = std::find(layers.begin(), layers.end(), layer_id); layer_it != layers.end())
			{
				return static_cast<AnimationLayerIndex>(*layer_it);
			}
		}

		return std::nullopt;
	}

	std::optional<AnimationRepository::AnimationLayerIndex> AnimationRepository::add_layer(AnimationLayerID layer_id)
	{
		if (!layer_id)
		{
			return std::nullopt;
		}

		if (layers.size() >= animation_layer_bits)
		{
			return std::nullopt;
		}

		auto layer_iterator = std::find(layers.cbegin(), layers.cend(), layer_id);

		if (layer_iterator == layers.cend())
		{
			layers.emplace_back(layer_id);

			layer_iterator = std::prev(layers.cend());
		}

		const auto layer_index = static_cast<AnimationLayerIndex>(std::distance(layers.cbegin(), layer_iterator));

		assert(layer_index < animation_layer_bits);

		return layer_index;
	}
}