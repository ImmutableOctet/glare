#pragma once

#include "reflection.hpp"

#include "behaviors.hpp"
//#include "meta.hpp"

#define GENERATE_EMPTY_BEHAVIOR_REFLECTION(behavior_type) \
    template <>                                           \
    void reflect<behavior_type>()                         \
    {                                                     \
        engine::behavior_meta_type<behavior_type>();      \
    }

namespace engine
{
	template <typename BehaviorType>
	auto behavior_meta_type()
	{
		return engine_meta_type<BehaviorType>()
			
			//.func<&BehaviorType::on_update>("on_update"_hs)
			//.func<&BehaviorType::on_update>("on_mouse"_hs)
			// ...

			// NOTE: `apply` is protected on some types; need to revisit this, or
			// conditionally handle the 'on_X' static member-functions.
			//.func<&BehaviorType::apply>("apply"_hs)
		;
	}

	GENERATE_EMPTY_BEHAVIOR_REFLECTION(BillboardBehavior);
	//GENERATE_EMPTY_BEHAVIOR_REFLECTION(RaveBehavior);

	template <>
	void reflect<SimpleFollowBehavior>()
	{
		behavior_meta_type<SimpleFollowBehavior>()
			.data<&SimpleFollowBehavior::leader>("leader"_hs)
			.data<&SimpleFollowBehavior::following_distance>("following_distance"_hs)
			.data<&SimpleFollowBehavior::follow_speed>("follow_speed"_hs)
			.data<&SimpleFollowBehavior::max_distance>("max_distance"_hs)

			.data<&SimpleFollowBehavior::set_force_catch_up, &SimpleFollowBehavior::get_force_catch_up>("force_catch_up"_hs)
			.data<&SimpleFollowBehavior::set_following, &SimpleFollowBehavior::get_following>("following"_hs)

			.ctor
			<
				decltype(SimpleFollowBehavior::leader),
				decltype(SimpleFollowBehavior::following_distance),
				decltype(SimpleFollowBehavior::follow_speed),
				decltype(SimpleFollowBehavior::max_distance),
				decltype(SimpleFollowBehavior::force_catch_up),
				decltype(SimpleFollowBehavior::following)
			>()

			.ctor
			<
				decltype(SimpleFollowBehavior::leader),
				decltype(SimpleFollowBehavior::following_distance),
				decltype(SimpleFollowBehavior::follow_speed),
				decltype(SimpleFollowBehavior::max_distance)
			>()
		;
	}

	template <>
	void reflect<RaveBehavior>()
	{
		behavior_meta_type<RaveBehavior>()
			// TODO: Deprecate.
			.data<&RaveBehavior::set_enabled, &RaveBehavior::get_enabled>("enabled"_hs)
		;
	}

	template <>
	void reflect<SpinBehavior>()
	{
		behavior_meta_type<SpinBehavior>()
			.data<&SpinBehavior::spin_vector>("spin_vector"_hs)
			.ctor<decltype(SpinBehavior::spin_vector)>()
		;
	}

	template <>
	void reflect<TargetBehavior>()
	{
		behavior_meta_type<TargetBehavior>()
			.data<&TargetBehavior::target>("target"_hs)
			.data<&TargetBehavior::interpolation>("interpolation"_hs)
			.data<&TargetBehavior::mode>("mode"_hs)

			.data<&TargetBehavior::set_allow_roll, &TargetBehavior::get_allow_roll>("allow_roll"_hs)

			// TODO: Deprecate.
			.data<&TargetBehavior::set_enabled, &TargetBehavior::get_enabled>("enabled"_hs)

			.ctor
			<
				decltype(TargetBehavior::target),
				decltype(TargetBehavior::interpolation),
				decltype(TargetBehavior::mode),
				decltype(TargetBehavior::allow_roll)
			>()
		;
	}

	template <>
	void reflect<FreeLookBehavior>()
	{
		behavior_meta_type<FreeLookBehavior>()
			.data<&FreeLookBehavior::sensitivity>("sensitivity"_hs)
			.ctor<decltype(FreeLookBehavior::sensitivity)>()
		;
	}

	template <>
	void reflect<DebugMoveBehavior>()
	{
		behavior_meta_type<DebugMoveBehavior>()
			.data<&DebugMoveBehavior::movement_speed>("movement_speed"_hs)
			.ctor<decltype(DebugMoveBehavior::movement_speed)>()
		;
	}

	void reflect_behaviors()
	{
		reflect<BillboardBehavior>();
		reflect<SimpleFollowBehavior>();
		reflect<RaveBehavior>();
		reflect<SpinBehavior>();
		reflect<TargetBehavior>();
		reflect<FreeLookBehavior>();
		reflect<DebugMoveBehavior>();
	}
}