#include "animation_layer.hpp"

namespace engine
{
	AnimationLayerMask compute_animation_layer_mask(AnimationLayerIndex layer_index)
	{
		assert(layer_index < MAX_ANIMATION_LAYERS);

		return static_cast<AnimationLayerMask>(static_cast<AnimationLayerMask>(1u) << static_cast<AnimationLayerMask>(layer_index));
	}

	AnimationLayerMask compute_mask_for_animation_layers(AnimationLayerCount layer_count)
	{
		assert(layer_count <= MAX_ANIMATION_LAYERS);

		return static_cast<AnimationLayerMask>((1u << layer_count) - 1);
	}
}