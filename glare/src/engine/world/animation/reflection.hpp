#pragma once

#include <engine/reflection.hpp>

#include "components/animation_component.hpp"
#include "components/skeletal_component.hpp"
#include "components/bone_component.hpp"

#include <graphics/animation.hpp>

namespace engine
{
	class AnimationSystem;

	template <>
	void reflect<AnimationComponent>()
	{
		engine_meta_type<AnimationComponent>()
			.data<&AnimationComponent::rate>("rate"_hs)
			.data<&AnimationComponent::time>("time"_hs)
			
			// Read-only properties:
			.data<nullptr, &AnimationComponent::get_state>("state"_hs)
			.data<nullptr, &AnimationComponent::paused>("paused"_hs)
			.data<nullptr, &AnimationComponent::playing>("playing"_hs)
			.data<nullptr, &AnimationComponent::transitioning>("transitioning"_hs)
			.data<nullptr, &AnimationComponent::animated>("animated"_hs)
			.data<nullptr, &AnimationComponent::pose_size>("pose_size"_hs)
			.data<nullptr, &AnimationComponent::get_animation_id>("animation_id"_hs)
			.data<nullptr, &AnimationComponent::get_current_animation>("current_animation"_hs)
			.data<nullptr, &AnimationComponent::get_prev_animation>("prev_animation"_hs)

			.func<&AnimationComponent::play>("play"_hs)
			.func<&AnimationComponent::pause>("pause"_hs)
			.func<&AnimationComponent::toggle>("toggle"_hs)
			.func<&AnimationComponent::get_animation>("get_animation_by_id"_hs)

			.ctor<decltype(AnimationComponent::rate)>()
		;
	}

	// TODO: Look into implications of reflecting this type without its factory function(s).
	template <>
	void reflect<SkeletalComponent>()
	{
		REFLECT_SINGLE_FIELD_COMPONENT(SkeletalComponent, root_bone);
	}

	// TODO: Look into implications of reflecting this type without its factory function(s).
	template <>
	void reflect<BoneComponent>()
	{
		engine_meta_type<BoneComponent>()
			.data<&BoneComponent::skeleton>("skeleton"_hs)
			.data<&BoneComponent::ID>("ID"_hs)
			.data<&BoneComponent::name>("name"_hs)
			.data<&BoneComponent::offset>("offset"_hs)

			.ctor
			<
				decltype(BoneComponent::skeleton),
				decltype(BoneComponent::ID),
				decltype(BoneComponent::name),
				decltype(BoneComponent::offset)
			>()

			//.ctor<&create_bone>()
		;
	}

	template <>
	void reflect<AnimationSystem>()
	{
		reflect<AnimationComponent>();
		reflect<SkeletalComponent>();
		reflect<BoneComponent>();
	}
}