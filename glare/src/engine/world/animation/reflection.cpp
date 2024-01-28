#pragma once

#include "reflection.hpp"

#include "animation_system.hpp"

#include "components/animation_component.hpp"
#include "components/skeletal_component.hpp"
#include "components/bone_component.hpp"

namespace engine
{
	template <>
	void reflect<AnimationComponent>()
	{
		engine_meta_type<AnimationComponent>()
			// TODO
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
			.data<&BoneComponent::bone_index>("bone_index"_hs)

			.ctor
			<
				decltype(BoneComponent::skeleton),
				decltype(BoneComponent::bone_index)
			>()

			//.ctor<&create_bone>()
		;
	}

	template <>
	void reflect<AnimationSystem>()
	{
		auto type = engine_system_type<AnimationSystem>()
			.func<&AnimationSystem::get_skeleton>("get_skeleton"_hs)
			.func<&AnimationSystem::get_frame_data>("get_frame_data"_hs)
			.func<&AnimationSystem::get_data_from_asset>("get_animation_data"_hs)
		;

		REFLECT_MEMBER_FUNCTION_OVERLOADS(type, AnimationSystem, play, 3, std::size_t, Registry&, Entity, AnimationID, AnimationLayerMask);
		REFLECT_MEMBER_FUNCTION_OVERLOADS(type, AnimationSystem, play, 3, std::size_t, Registry&, Entity, std::string_view, AnimationLayerMask);

		reflect<AnimationComponent>();
		reflect<SkeletalComponent>();
		reflect<BoneComponent>();
	}
}