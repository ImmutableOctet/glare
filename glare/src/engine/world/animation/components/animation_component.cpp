#include "animation_component.hpp"

#include <engine/world/animation/animation_layer.hpp>

#include <util/bitfield.hpp>

#include <algorithm>

namespace engine
{
	namespace impl
	{
		template <typename Callback>
		static std::size_t modify_animation_layers_impl
		(
			AnimationComponent::LayerContainer& layers,
			AnimationLayerMask layers_affected,
			Callback&& callback
		)
		{
			auto layers_modified = std::size_t {};

			auto on_layer = [&](auto layer_index)
			{
				auto& layer = layers[layer_index];

				if (callback(layer))
				{
					layers_modified++;
				}
			};

			if (layers_affected == ANIMATION_LAYER_MASK_NO_LAYER)
			{
				on_layer(AnimationComponent::ANIMATION_LAYER_INDEX_NO_LAYER);
			}
			else
			{
				util::enumerate_enabled_bits(layers_affected, on_layer);
			}

			return layers_modified;
		}
	}

	std::size_t AnimationComponent::play
	(
		AnimationID animation_id,
		AnimationSlice animation_slice,
		AnimationLayerMask animation_layers
	)
	{
		return impl::modify_animation_layers_impl
		(
			this->layers,

			animation_layers,
			
			[&](auto& layer)
			{
				return layer.play(animation_id, animation_slice);
			}
		);
	}

	std::size_t AnimationComponent::pause(AnimationLayerMask animation_layers)
	{
		return impl::modify_animation_layers_impl
		(
			this->layers,

			animation_layers,
			
			[&](auto& layer)
			{
				return layer.pause();
			}
		);
	}

	std::size_t AnimationComponent::stop(AnimationLayerMask animation_layers)
	{
		return impl::modify_animation_layers_impl
		(
			this->layers,

			animation_layers,
			
			[&](auto& layer)
			{
				return layer.stop();
			}
		);
	}

	bool AnimationComponent::has_playing_layer() const
	{
		return (std::find_if(layers.begin(), layers.end(), [](const auto& layer) { return layer.playing(); }) != layers.end());
	}

	bool AnimationComponent::has_paused_layer() const
	{
		return (std::find_if(layers.begin(), layers.end(), [](const auto& layer) { return layer.paused(); }) != layers.end());
	}

	bool AnimationComponent::has_stopped_layer() const
	{
		return (std::find_if(layers.begin(), layers.end(), [](const auto& layer) { return layer.stopped(); }) != layers.end());
	}

	std::size_t AnimationComponent::layers_playing() const
	{
		return std::count_if(layers.begin(), layers.end(), [](const auto& layer) { return layer.playing(); });
	}

	std::size_t AnimationComponent::layers_paused() const
	{
		return std::count_if(layers.begin(), layers.end(), [](const auto& layer) { return layer.paused(); });
	}

	std::size_t AnimationComponent::layers_stopped() const
	{
		return std::count_if(layers.begin(), layers.end(), [](const auto& layer) { return layer.stopped(); });
	}

	AnimationLayerMask AnimationComponent::layers_playing_mask() const
	{
		return generate_layer_mask([](const auto& layer) { return layer.playing(); });
	}

	AnimationLayerMask AnimationComponent::layers_paused_mask() const
	{
		return generate_layer_mask([](const auto& layer) { return layer.paused(); });
	}

	AnimationLayerMask AnimationComponent::layers_stopped_mask() const
	{
		return generate_layer_mask([](const auto& layer) { return layer.stopped(); });
	}
}