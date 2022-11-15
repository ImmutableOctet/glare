#pragma once

#include <engine/reflection.hpp>

#include "components/alignment_component.hpp"
#include "components/gravity_component.hpp"
#include "components/ground_component.hpp"
#include "components/motion_attachment_proxy_component.hpp"
#include "components/motion_component.hpp"
#include "components/velocity_component.hpp"
#include "components/deceleration_component.hpp"

namespace engine
{
	class MotionSystem;

	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(AlignmentComponent, entity);
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
	void reflect<MotionComponent>()
	{
		engine_meta_type<MotionComponent>();
	}

	template <>
	void reflect<MotionSystem>()
	{
		reflect<MotionComponent>();
		reflect<AlignmentComponent>();
		reflect<VelocityComponent>();
		reflect<GravityComponent>();
		reflect<DecelerationComponent>();
		reflect<GroundComponent>();
		reflect<MotionAttachmentProxyComponent>();
	}
}