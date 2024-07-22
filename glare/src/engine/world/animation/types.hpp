#pragma once

#include <engine/types.hpp>

#include <graphics/types.hpp>

// TODO: Look into removing this from the main 'types' header.
namespace graphics
{
	struct AnimationData;
}

namespace engine
{
	using AnimationID         = entt::id_type; // graphics::AnimationID;
	using AnimationLayerID    = entt::id_type;
	using BoneID              = entt::id_type; // graphics::BoneID;
	using BoneIndex           = graphics::BoneIndex;

	// A type representing a bitmask used to filter which bones an animation is applied to.
	using AnimationLayerMask  = std::uint8_t;

	// Index type used for animation layers.
	using AnimationLayerIndex = std::size_t;

	using AnimationLayerCount = AnimationLayerIndex;

	using FrameIndex          = std::uint32_t;
	using FrameSliceSize      = FrameIndex;

	inline constexpr FrameIndex INVALID_FRAME_INDEX = {};

	inline constexpr AnimationLayerMask ANIMATION_LAYER_MASK_NO_LAYER = {};

	// Maximum number of concurrent animation layers. (Number of bits)
	inline constexpr std::size_t MAX_ANIMATION_LAYERS = (sizeof(AnimationLayerMask) * 8);
}