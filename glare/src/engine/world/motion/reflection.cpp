#pragma once

#include "reflection.hpp"

#include "components/gravity_component.hpp"
#include "components/ground_component.hpp"
#include "components/alignment_proxy_component.hpp"
#include "components/motion_attachment_proxy_component.hpp"
#include "components/motion_component.hpp"
#include "components/velocity_component.hpp"
#include "components/deceleration_component.hpp"
#include "components/focus_component.hpp"
#include "components/orbit_component.hpp"

#include "motion_events.hpp"

namespace engine
{
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(AlignmentProxyComponent, entity);
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(VelocityComponent, velocity);
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(GravityComponent, intensity); //GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(GravityComponent, ...);
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(DecelerationComponent, deceleration);

	template <>
	void reflect<GroundComponent>()
	{
		engine_meta_type<GroundComponent>()
			//.data<&GroundComponent::detection_threshold>("detection_threshold"_hs)
			.data<nullptr, &GroundComponent::get_detection_threshold>("detection_threshold"_hs)
			.data<&GroundComponent::ground>("ground"_hs)
			.data<nullptr, &GroundComponent::entity>("entity"_hs)
			.data<nullptr, &GroundComponent::on_ground>("on_ground"_hs)
			.ctor
			<
				float, //decltype(GroundComponent::detection_threshold),
				decltype(GroundComponent::ground)
			>()
		;
	}

	template <>
	void reflect<MotionAttachmentProxyComponent>()
	{
		//REFLECT_SINGLE_FIELD_COMPONENT(MotionAttachmentProxyComponent, intended_parent);

		engine_meta_type<MotionAttachmentProxyComponent>()
			.data<&MotionAttachmentProxyComponent::intended_parent>("intended_parent"_hs)
			.data<nullptr, &MotionAttachmentProxyComponent::is_active>("is_active"_hs)
		;
	}

	template <>
	template <>
	void reflect<FocusComponent>()
	{
		engine_meta_type<FocusComponent>()
			.data<&FocusComponent::target>("target"_hs)
			.data<&FocusComponent::focus_offset>("focus_offset"_hs)
			.data<&FocusComponent::tracking_speed>("tracking_speed"_hs)
		;
	}

	template <>
	void reflect<OrbitComponent>()
	{
		engine_meta_type<OrbitComponent>()
			.data<&OrbitComponent::movement_speed>("movement_speed"_hs)
			.data<&OrbitComponent::distance>("distance"_hs)
			.data<&OrbitComponent::min_distance>("min_distance"_hs)
			.data<&OrbitComponent::max_distance>("max_distance"_hs)
		;
	}

	template <>
	void reflect<MotionComponent>()
	{
		// TODO: Add getter-methods for boolean data members.
		engine_meta_type<MotionComponent>();
	}

	template <>
	void reflect<OnAirToGround>()
	{
		engine_meta_type<OnAirToGround>()
			.data<&OnAirToGround::surface>("surface"_hs)
			.data<nullptr, &OnAirToGround::landing_vector>("landing_vector"_hs)
			.data<nullptr, &OnAirToGround::ground>("ground"_hs)
			.data<nullptr, &OnAirToGround::ground_position>("ground_position"_hs)
			.data<nullptr, &OnAirToGround::is_dynamic_ground>("is_dynamic_ground"_hs)
			.ctor<decltype(OnAirToGround::surface)>()
		;
	}

	template <>
	void reflect<OnGroundToAir>()
	{
		engine_meta_type<OnGroundToAir>()
			.data<&OnGroundToAir::surface>("surface"_hs)
			.data<&OnGroundToAir::entity_position>("entity_position"_hs)
			.data<&OnGroundToAir::escape_vector>("escape_vector"_hs)
			.data<nullptr, &OnGroundToAir::ground>("ground"_hs)
			.data<nullptr, &OnGroundToAir::was_dynamic_ground>("was_dynamic_ground"_hs)
			.ctor
			<
				decltype(OnGroundToAir::surface),
				decltype(OnGroundToAir::entity_position),
				decltype(OnGroundToAir::escape_vector)
			>()
		;
	}

	template <>
	void reflect<OnMotionAttachment>()
	{
		engine_meta_type<OnMotionAttachment>()
			.data<&OnMotionAttachment::attached_to>("attached_to"_hs)
			.ctor<decltype(OnMotionAttachment::entity), decltype(OnMotionAttachment::attached_to)>()
		;
	}

	template <>
	void reflect<OnMotionDetachment>()
	{
		engine_meta_type<OnMotionDetachment>()
			.data<&OnMotionDetachment::detached_from>("detached_from"_hs)
			.ctor<decltype(OnMotionDetachment::entity), decltype(OnMotionDetachment::detached_from)>()
		;
	}

	template <>
	void reflect<MotionSystem>()
	{
		// Components:
		reflect<MotionComponent>();
		reflect<AlignmentProxyComponent>();
		reflect<VelocityComponent>();
		reflect<GravityComponent>();
		reflect<DecelerationComponent>();
		reflect<GroundComponent>();
		reflect<MotionAttachmentProxyComponent>();
		reflect<FocusComponent>();
		reflect<OrbitComponent>();

		// Events:
		reflect<OnAirToGround>();
		reflect<OnGroundToAir>();
		reflect<OnMotionAttachment>();
		reflect<OnMotionDetachment>();
	}
}