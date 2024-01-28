#pragma once

#include <engine/world/animation/types.hpp>
#include <engine/world/animation/animation_layer.hpp>

#include <array>

#include <cstddef>
#include <cassert>

namespace engine
{
	struct AnimationComponent
	{
		// NOTE: We preallocate to `MAX_ANIMATION_LAYERS` + 1 to account for the primary layer.
		using LayerContainer = std::array<AnimationLayer, (MAX_ANIMATION_LAYERS + 1)>;

		inline static constexpr AnimationLayerIndex ANIMATION_LAYER_INDEX_NO_LAYER = static_cast<AnimationLayerIndex>(MAX_ANIMATION_LAYERS);

		// Animation layers currently being applied.
		LayerContainer layers = {};

		std::size_t play
		(
			AnimationID animation_id,
			AnimationSlice animation_slice,
			AnimationLayerMask animation_layers=ANIMATION_LAYER_MASK_NO_LAYER
		);

		std::size_t pause(AnimationLayerMask animation_layers=ANIMATION_LAYER_MASK_NO_LAYER);

		std::size_t stop(AnimationLayerMask animation_layers=ANIMATION_LAYER_MASK_NO_LAYER);

		bool has_playing_layer() const;
		bool has_paused_layer() const;
		bool has_stopped_layer() const;

		std::size_t layers_playing() const;
		std::size_t layers_paused() const;
		std::size_t layers_stopped() const;

		AnimationLayerMask layers_playing_mask() const;
		AnimationLayerMask layers_paused_mask() const;
		AnimationLayerMask layers_stopped_mask() const;

		constexpr AnimationLayer& get_layer(AnimationLayerIndex layer_index)
		{
			assert(layer_index < layers.size());

			return layers[layer_index];
		}

		constexpr const AnimationLayer& get_layer(AnimationLayerIndex layer_index) const
		{
			assert(layer_index < layers.size());

			return layers[layer_index];
		}

		constexpr AnimationLayer& get_primary_layer()
		{
			return get_layer(ANIMATION_LAYER_INDEX_NO_LAYER);
		}

		constexpr const AnimationLayer& get_primary_layer() const
		{
			return get_layer(ANIMATION_LAYER_INDEX_NO_LAYER);
		}

		constexpr const AnimationLayerData& get_primary_animation_data() const
		{
			return get_primary_layer().current;
		}

		constexpr AnimationID get_primary_animation_id() const
		{
			return get_primary_animation_data().name;
		}

		constexpr AnimationSlice get_primary_animation_area() const
		{
			return get_primary_animation_data().area;
		}

		constexpr const AnimationState& get_primary_animation_state() const
		{
			return get_primary_animation_data().state;
		}

		float get_primary_animation_time() const
		{
			return get_primary_animation_state().get_time();
		}

		inline bool playing() const
		{
			return has_playing_layer();
		}

		template <typename Callback>
		AnimationLayerMask generate_layer_mask(Callback&& callback) const
		{
			auto layer_mask = ANIMATION_LAYER_MASK_NO_LAYER;

			for (auto layer_index = AnimationLayerIndex {}; layer_index < static_cast<AnimationLayerIndex>(layers.size()); layer_index++)
			{
				const auto& layer = get_layer(layer_index);

				if (callback(layer))
				{
					layer_mask |= compute_animation_layer_mask(layer_index);
				}
			}

			return layer_mask;
		}

		explicit operator bool() const
		{
			return playing();
		}
	};
}