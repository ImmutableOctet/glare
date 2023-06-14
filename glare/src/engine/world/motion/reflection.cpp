#pragma once

#include "reflection.hpp"

#include "motion_system.hpp"

#include "components/gravity_component.hpp"
#include "components/ground_component.hpp"
#include "components/alignment_proxy_component.hpp"
#include "components/motion_attachment_proxy_component.hpp"
#include "components/motion_component.hpp"
#include "components/velocity_component.hpp"
#include "components/acceleration_component.hpp"
#include "components/deceleration_component.hpp"
#include "components/directional_influence_component.hpp"
#include "components/direction_component.hpp"
#include "components/orientation_component.hpp"
#include "components/rotate_component.hpp"
#include "components/focus_component.hpp"
#include "components/orbit_component.hpp"

#include "motion_events.hpp"

#include <engine/transform.hpp>

namespace engine
{
	//struct Transform;

	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(AlignmentProxyComponent, entity);
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(GravityComponent, intensity); //GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(GravityComponent, ...);
	
	template <>
	void reflect<VelocityComponent>()
	{
		engine_meta_type<VelocityComponent>()
			.data<&VelocityComponent::velocity>("velocity"_hs)

			.data<nullptr, &VelocityComponent::speed>("speed"_hs)
			.data<nullptr, &VelocityComponent::direction>("direction"_hs)

			.conv<math::Vector>()
		;
	}

	template <>
	void reflect<AccelerationComponent>()
	{
		engine_meta_type<AccelerationComponent>()
			.data<&AccelerationComponent::ground>("ground"_hs)
			.data<&AccelerationComponent::air>("air"_hs)
		;
	}

	template <>
	void reflect<DecelerationComponent>()
	{
		engine_meta_type<DecelerationComponent>()
			.data<&DecelerationComponent::ground>("ground"_hs)
			.data<&DecelerationComponent::air>("air"_hs)
		;
	}

	template <>
	void reflect<DirectionalInfluenceComponent>()
	{
		engine_meta_type<DirectionalInfluenceComponent>()
			.data<&DirectionalInfluenceComponent::ground>("ground"_hs)
			.data<&DirectionalInfluenceComponent::air>("air"_hs)
		;
	}

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
	void reflect<DirectionComponent>()
	{
		engine_meta_type<DirectionComponent>()
			.data<&DirectionComponent::direction>("direction"_hs)
			.data<&DirectionComponent::turn_speed>("turn_speed"_hs)
			.data<&DirectionComponent::set_ignore_x, &DirectionComponent::get_ignore_x>("ignore_x"_hs)
			.data<&DirectionComponent::set_ignore_y, &DirectionComponent::get_ignore_y>("ignore_y"_hs)
			.data<&DirectionComponent::set_ignore_z, &DirectionComponent::get_ignore_z>("ignore_z"_hs)

			.ctor
			<
				decltype(DirectionComponent::direction)
			>()

			.ctor
			<
				decltype(DirectionComponent::direction),
				decltype(DirectionComponent::turn_speed)
			>()

			.ctor
			<
				decltype(DirectionComponent::direction),
				decltype(DirectionComponent::turn_speed),
				decltype(DirectionComponent::ignore_x)
			>()

			.ctor
			<
				decltype(DirectionComponent::direction),
				decltype(DirectionComponent::turn_speed),
				decltype(DirectionComponent::ignore_x),
				decltype(DirectionComponent::ignore_y)
			>()

			.ctor
			<
				decltype(DirectionComponent::direction),
				decltype(DirectionComponent::turn_speed),
				decltype(DirectionComponent::ignore_x),
				decltype(DirectionComponent::ignore_y),
				decltype(DirectionComponent::ignore_z)
			>()
		;
	}

	template <>
	void reflect<OrientationComponent>()
	{
		auto type = engine_meta_type<OrientationComponent>()
			.data<&OrientationComponent::orientation>("orientation"_hs)
			.data<&OrientationComponent::turn_speed>("turn_speed"_hs)
			.data<&OrientationComponent::set_direction, &OrientationComponent::get_direction>("direction"_hs)

			.ctor
			<
				decltype(OrientationComponent::orientation)
			>()

			.ctor
			<
				decltype(OrientationComponent::orientation),
				decltype(OrientationComponent::turn_speed)
			>()
		;

		type = make_constructors
		<
			&OrientationComponent::from_direction,
			[](auto&&... args) { return OrientationComponent::from_direction(std::forward<decltype(args)>(args)...); },
			1
		>(type);
	}

	template <>
	void reflect<RotateComponent>()
	{
		auto type = engine_meta_type<RotateComponent>()
			.data<&RotateComponent::relative_orientation>("relative_orientation"_hs)
			.data<&RotateComponent::turn_speed>("turn_speed"_hs)
			.data<&RotateComponent::set_use_local_rotation, &RotateComponent::get_use_local_rotation>("use_local_rotation"_hs)

			//.data<&RotateComponent::set_direction, nullptr>("direction"_hs)

			.func<&RotateComponent::set_direction>("set_direction"_hs)
			.func<&RotateComponent::get_direction>("get_direction"_hs)

			.func<&RotateComponent::get_next_basis>("get_next_basis"_hs)

			.func<static_cast<math::Vector(RotateComponent::*)(const Transform&, float) const>(&RotateComponent::get_next_direction)>("get_next_direction"_hs)
			.func<static_cast<math::Vector(RotateComponent::*)(Entity, Registry&, float) const>(&RotateComponent::get_next_direction)>("get_next_direction"_hs)

			.ctor
			<
				decltype(RotateComponent::relative_orientation)
			>()

			.ctor
			<
				decltype(RotateComponent::relative_orientation),
				decltype(RotateComponent::turn_speed)
			>()

			.ctor
			<
				decltype(RotateComponent::relative_orientation),
				decltype(RotateComponent::turn_speed),
				decltype(RotateComponent::use_local_rotation)
			>()
		;

		type = make_constructors
		<
			&RotateComponent::from_direction,
			[](auto&&... args) { return RotateComponent::from_direction(std::forward<decltype(args)>(args)...); },
			1
		>(type);
	}

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
		auto motion = engine_system_type<MotionSystem>()
			.func<&MotionSystem::accelerate>("accelerate"_hs)
			.func<&MotionSystem::influence_motion_direction>("influence_motion_direction"_hs)
		;

		// Components:
		reflect<MotionComponent>();
		reflect<AlignmentProxyComponent>();
		reflect<VelocityComponent>();
		reflect<GravityComponent>();
		reflect<AccelerationComponent>();
		reflect<DecelerationComponent>();
		reflect<DirectionalInfluenceComponent>();
		reflect<GroundComponent>();
		reflect<MotionAttachmentProxyComponent>();
		reflect<DirectionComponent>();
		reflect<OrientationComponent>();
		reflect<RotateComponent>();
		reflect<FocusComponent>();
		reflect<OrbitComponent>();

		// Events:
		reflect<OnAirToGround>();
		reflect<OnGroundToAir>();
		reflect<OnMotionAttachment>();
		reflect<OnMotionDetachment>();
	}
}