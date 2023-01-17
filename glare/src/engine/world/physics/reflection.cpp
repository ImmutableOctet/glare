#pragma once

#include "reflection.hpp"

#include "components/collision_component.hpp"

#include "ground.hpp"
//#include "collision_surface.hpp"

#include <engine/world/world.hpp>

// TODO: Reflect other types utilized by the physics system.

namespace engine
{
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
		;
	}

	// TODO: Implement a formal reflected interface for `PhysicsSystem`.
	template <>
	void reflect<PhysicsSystem>()
	{
		//reflect<CollisionSurface>();
		reflect<Ground>();
		reflect<CollisionComponent>();
	}
}