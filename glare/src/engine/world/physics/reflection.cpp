#pragma once

#include "reflection.hpp"
#include "types.hpp"
#include "physics_system.hpp"
#include "collision_cast.hpp"
#include "collision_shape_description.hpp"
#include "kinematic_resolution_config.hpp"

#include "components/collision_component.hpp"

#include "ground.hpp"
//#include "collision_surface.hpp"

#include <engine/world/world.hpp>

// TODO: Reflect other types utilized by the physics system.

namespace engine
{
	template <>
	void reflect<CollisionCastResult>()
	{
		using native_t = decltype(CollisionCastResult::native);

		engine_meta_type<CollisionCastResult>()
			.data<&CollisionCastResult::cast_entity>("cast_entity"_hs)
			.data<&CollisionCastResult::hit_entity>("hit_entity"_hs)
			.data<&CollisionCastResult::hit_position>("hit_position"_hs)
			.data<&CollisionCastResult::hit_normal>("hit_normal"_hs)
			.data<&CollisionCastResult::closest_hit_fraction>("closest_hit_fraction"_hs)

			.data<&CollisionCastResult::native>("native"_hs)

			//.data<&CollisionCastResult::hit_entity>("entity"_hs) // cast_entity
		;

		engine_meta_type<native_t>()
			.data<&native_t::cast_object>("cast_object"_hs)
			.data<&native_t::hit_object>("hit_object"_hs)
		;
	}

	template <>
	void reflect<ConvexCastResult>()
	{
		engine_meta_type<ConvexCastResult>()
			.base<CollisionCastResult>()
		;
	}

	template <>
	void reflect<RayCastResult>()
	{
		engine_meta_type<RayCastResult>()
			.base<CollisionCastResult>()
		;
	}

	template <>
	void reflect<Ground>()
	{
		engine_meta_type<Ground>()
			.data<&Ground::set_is_dynamic, &Ground::get_is_dynamic>("is_dynamic"_hs)
			.data<&Ground::set_is_contacted, &Ground::get_is_contacted>("is_contacted"_hs)
			.data<nullptr, &Ground::contacting_entity>("contacting_entity"_hs)
			.data<nullptr, &Ground::entity>("entity"_hs)
			.data<nullptr, &Ground::collision_object>("collision_object"_hs)

			.ctor<const CollisionSurface&>()
			.ctor<World&, const CollisionSurface&>()
		;
	}

	template <>
	void reflect<CollisionComponent>()
	{
		engine_meta_type<CollisionComponent>()
			.data<nullptr, &CollisionComponent::is_active>("active"_hs)
			.data<nullptr, &CollisionComponent::is_kinematic>("kinematic"_hs)
			.data<nullptr, &CollisionComponent::is_dynamic>("dynamic"_hs)
			.data<nullptr, &CollisionComponent::is_static>("static"_hs)
			.data<nullptr, &CollisionComponent::is_convex>("convex"_hs)
			
			.data<&CollisionComponent::set_mass, &CollisionComponent::get_mass>("mass"_hs)

			// TODO: Look at reflecting these again:
			//.data<nullptr, &CollisionComponent::get_kinematic_resolution>("kinematic_cast_method"_hs)
			//.data<&CollisionComponent::set_kinematic_resolution, &CollisionComponent::get_kinematic_resolution>("kinematic_resolution"_hs)

			.data<nullptr, &CollisionComponent::get_collision_object_transform>("collision_object_transform"_hs)
			.data<nullptr, &CollisionComponent::get_collision_object_orientation>("collision_object_orientation"_hs)

			.data<nullptr, &CollisionComponent::get_group>("group"_hs)
			.data<nullptr, &CollisionComponent::get_interactions>("interactions"_hs)
			.data<nullptr, &CollisionComponent::get_solids>("solids"_hs)

			.data<nullptr, &CollisionComponent::get_shape>("shape"_hs)
			.data<nullptr, &CollisionComponent::get_convex_shape>("convex_shape"_hs)
			.data<nullptr, &CollisionComponent::get_concave_shape>("concave_shape"_hs)

			.data<nullptr, static_cast<const btCollisionObject* (CollisionComponent::*)() const>(&CollisionComponent::get_collision_object)>("collision_object"_hs)
			.data<nullptr, static_cast<const btRigidBody* (CollisionComponent::*)() const>(&CollisionComponent::get_rigid_body)>("rigid_body"_hs)

			.func<&CollisionComponent::get_entity_from_collision_object>("get_entity_from_collision_object"_hs)

			.func<&CollisionComponent::activate>("activate"_hs)
			.func<&CollisionComponent::deactivate>("deactivate"_hs)

			.func<&CollisionComponent::has_collision_object>("has_collision_object"_hs)
			.func<&CollisionComponent::has_collision_object>("has_kinematic_cast_method"_hs)

			.data<nullptr, &CollisionComponent::get_world_aabb>("world_aabb"_hs)
			.data<nullptr, &CollisionComponent::get_world_bounding_sphere>("world_bounding_sphere"_hs)
			.data<nullptr, &CollisionComponent::get_aabb_lengths>("aabb_lengths"_hs)
			.data<nullptr, &CollisionComponent::get_aabb_size>("aabb_size"_hs)
			.data<nullptr, &CollisionComponent::get_bounding_radius>("bounding_radius"_hs)
			.data<nullptr, &CollisionComponent::get_inner_radius>("inner_radius"_hs)
			.data<nullptr, &CollisionComponent::get_inner_diameter>("inner_diameter"_hs)

			.ctor
			<
				const CollisionShapeDescription&,
				const CollisionConfig&,
				const KinematicResolutionConfig&,
				CollisionBodyType,
				float
			>()

			.ctor
			<
				const CollisionShapeDescription&,
				const CollisionConfig&,
				const KinematicResolutionConfig&,
				CollisionBodyType
			>()

			.ctor
			<
				const CollisionShapeDescription&,
				const CollisionConfig&,
				const KinematicResolutionConfig&
			>()

			.ctor
			<
				const CollisionShapeDescription&,
				const CollisionConfig&
			>()

			.ctor
			<
				const CollisionShapeDescription&,
				EntityType
			>()

			.ctor
			<
				const CollisionShapeDescription&,
				EntityType,
				const KinematicResolutionConfig&
			>()
		;
	}

	template <>
	void reflect<CollisionShapeDescription>()
	{
		engine_meta_type<CollisionShapeDescription>()
			.data<&CollisionShapeDescription::primitive>("primitive"_hs)
			.data<&CollisionShapeDescription::size>("size"_hs)

			.data<&CollisionShapeDescription::set_radius,  &CollisionShapeDescription::get_radius>("radius"_hs)
			.data<&CollisionShapeDescription::set_height,  &CollisionShapeDescription::get_height>("height"_hs)
			.data<&CollisionShapeDescription::set_xz_size, &CollisionShapeDescription::get_xz_size>("xz_size"_hs)
		;
	}

	template <>
	void reflect<KinematicResolutionConfig>()
	{
		engine_empty_meta_type<KinematicResolutionConfig::SizeConfig>();

		// Automatic size types:
		engine_empty_meta_type<KinematicResolutionConfig::AABBType>();
		engine_empty_meta_type<KinematicResolutionConfig::AABBVectorType>();
		engine_empty_meta_type<KinematicResolutionConfig::OrientedAABBVectorType>();
		engine_empty_meta_type<KinematicResolutionConfig::SphereType>();
		engine_empty_meta_type<KinematicResolutionConfig::InnerSphereType>();

		// Manual size types:
		auto manual_size_type = []<typename SizeType>()
		{
			engine_meta_type<SizeType>()
				.data<&SizeType::set_size, &SizeType::get_size>("size"_hs)
				.data<&SizeType::set_half_size, &SizeType::get_half_size>("half_size"_hs)
				.ctor<typename SizeType::ValueType>();
			;
		};

		manual_size_type.operator()<KinematicResolutionConfig::SizeType>();
		manual_size_type.operator()<KinematicResolutionConfig::VectorSizeType>();
		manual_size_type.operator()<KinematicResolutionConfig::OrientedVectorSizeType>();

		// KinematicResolutionConfig:
		auto kinematic_resolution_type = engine_meta_type<KinematicResolutionConfig>()
			.data<&KinematicResolutionConfig::cast_method>("cast_method"_hs)
			.data<&KinematicResolutionConfig::size>("size"_hs)

			.data<&KinematicResolutionConfig::set_is_influencer, &KinematicResolutionConfig::get_is_influencer>("is_influencer"_hs)
			.data<&KinematicResolutionConfig::set_accepts_influence, &KinematicResolutionConfig::get_accepts_influence>("accepts_influence"_hs)
			.data<&KinematicResolutionConfig::set_resolve_intersections, &KinematicResolutionConfig::get_resolve_intersections>("resolve_intersections"_hs)
			.data<&KinematicResolutionConfig::set_can_influence_children, &KinematicResolutionConfig::get_can_influence_children>("can_influence_children"_hs)
			.data<&KinematicResolutionConfig::set_can_be_influenced_by_children, &KinematicResolutionConfig::get_can_be_influenced_by_children>("can_be_influenced_by_children"_hs)

			.func<&KinematicResolutionConfig::get_size>("get_size"_hs)
			.func<&KinematicResolutionConfig::get_half_size>("get_half_size"_hs)
			.func<&KinematicResolutionConfig::get_size_vector>("get_size_vector"_hs)
			.func<&KinematicResolutionConfig::get_half_size_vector>("get_half_size_vector"_hs)

			.ctor<decltype(KinematicResolutionConfig::cast_method)>()
		;

		// Constructor generator:
		auto size_type = [&kinematic_resolution_type]<typename SizeType>()
		{
			kinematic_resolution_type = kinematic_resolution_type
			.ctor
			<
				decltype(KinematicResolutionConfig::cast_method),
				SizeType,

				decltype(KinematicResolutionConfig::is_influencer),
				decltype(KinematicResolutionConfig::accepts_influence),
				decltype(KinematicResolutionConfig::resolve_intersections),
				decltype(KinematicResolutionConfig::can_influence_children),
				decltype(KinematicResolutionConfig::can_be_influenced_by_children)
			>()
			.ctor
			<
				decltype(KinematicResolutionConfig::cast_method),
				SizeType,

				decltype(KinematicResolutionConfig::is_influencer),
				decltype(KinematicResolutionConfig::accepts_influence),
				decltype(KinematicResolutionConfig::resolve_intersections),
				decltype(KinematicResolutionConfig::can_influence_children)
			>()
			.ctor
			<
				decltype(KinematicResolutionConfig::cast_method),
				SizeType,

				decltype(KinematicResolutionConfig::is_influencer),
				decltype(KinematicResolutionConfig::accepts_influence),
				decltype(KinematicResolutionConfig::resolve_intersections)
			>()
			.ctor
			<
				decltype(KinematicResolutionConfig::cast_method),
				SizeType,

				decltype(KinematicResolutionConfig::is_influencer),
				decltype(KinematicResolutionConfig::accepts_influence)
			>()
			.ctor
			<
				decltype(KinematicResolutionConfig::cast_method),
				SizeType,

				decltype(KinematicResolutionConfig::is_influencer)
			>()
			.ctor
			<
				decltype(KinematicResolutionConfig::cast_method),
				SizeType
			>()
			;
		};

		// Automatic size types:
		size_type.operator()<KinematicResolutionConfig::AABBType>();
		size_type.operator()<KinematicResolutionConfig::AABBVectorType>();
		size_type.operator()<KinematicResolutionConfig::OrientedAABBVectorType>();
		size_type.operator()<KinematicResolutionConfig::SphereType>();
		size_type.operator()<KinematicResolutionConfig::InnerSphereType>();

		// Manual size types:
		size_type.operator()<KinematicResolutionConfig::SizeType>();
		size_type.operator()<KinematicResolutionConfig::VectorSizeType>();
		size_type.operator()<KinematicResolutionConfig::OrientedVectorSizeType>();
	}

	// TODO: Implement a formal reflected interface for `PhysicsSystem`.
	template <>
	void reflect<PhysicsSystem>()
	{
		engine_system_type<PhysicsSystem>()
			.func<&PhysicsSystem::cast_to<btCollisionObject>>("cast_to"_hs)
			.func<&PhysicsSystem::cast_to<Entity>>("cast_to"_hs)
			
			//.func<&PhysicsSystem::cast_to<CollisionComponent>>("cast_to"_hs)

			.func
			<
				static_cast
				<
					std::optional<CollisionCastResult> (PhysicsSystem::*)
					(
						const CollisionComponent&, const math::Vector&,
						std::optional<CollisionGroup>, std::optional<CollisionGroup>,
						bool
					)
				>(&PhysicsSystem::cast_to)
			>("cast_to"_hs)

			.func<&convex_cast_to<CollisionComponent>>("convex_cast_to"_hs)
			.func<&convex_cast_to<Entity>>("convex_cast_to"_hs)
			.func<&convex_cast_to<CollisionObjectAndConvexShape>>("convex_cast_to"_hs)
			
			.func<&ray_cast<btCollisionObject>>("ray_cast"_hs)
			.func<&ray_cast<CollisionComponent>>("ray_cast"_hs)
			.func<&ray_cast<Entity>>("ray_cast"_hs)

			.func
			<
				static_cast
				<
					std::optional<RayCastResult> (*)
					(
						PhysicsSystem&,

						const math::Vector&,
						const math::Vector&,

						std::optional<CollisionGroup>,
						std::optional<CollisionGroup>
					)
				>
				(&ray_cast)
			>("ray_cast"_hs)
			
			.func<&directional_ray_cast<btCollisionObject>>("directional_ray_cast"_hs)
			.func<&directional_ray_cast<CollisionComponent>>("directional_ray_cast"_hs)
			.func<&directional_ray_cast<Entity>>("directional_ray_cast"_hs)

			.func
			<
				static_cast
				<
					std::optional<RayCastResult> (*)
					(
						PhysicsSystem&,

						const math::Vector&,
						const math::Vector&,

						std::optional<float>,

						std::optional<CollisionGroup>,
						std::optional<CollisionGroup>
					)
				>
				(&directional_ray_cast)
			>("directional_ray_cast"_hs)
			
			.func<&ray_cast_to<btCollisionObject>>("ray_cast_to"_hs)
			.func<&ray_cast_to<CollisionComponent>>("ray_cast_to"_hs)
			.func<&ray_cast_to<Entity>>("ray_cast_to"_hs)

			.func<&directional_ray_cast_to<btCollisionObject>>("directional_ray_cast_to"_hs)
			.func<&directional_ray_cast_to<CollisionComponent>>("directional_ray_cast_to"_hs)
			.func<&directional_ray_cast_to<Entity>>("directional_ray_cast_to"_hs)

			.func<&convex_cast>("convex_cast"_hs)

			.data<nullptr, &PhysicsSystem::get_collision_world>("collision_world"_hs)
			.data<nullptr, &PhysicsSystem::get_broadphase>("broadphase"_hs)
			.data<nullptr, &PhysicsSystem::get_collision_dispatcher>("collision_dispatcher"_hs)
			.data<nullptr, &PhysicsSystem::get_max_ray_distance>("max_ray_distance"_hs)
		;

		reflect<CollisionShapePrimitive>();
		reflect<CollisionCastMethod>();

		reflect<CollisionCastResult>();
		reflect<ConvexCastResult>();
		reflect<RayCastResult>();

		reflect<CollisionShapeDescription>();
		reflect<KinematicResolutionConfig>();

		//reflect<CollisionSurface>();
		reflect<Ground>();
		reflect<CollisionComponent>();
	}
}