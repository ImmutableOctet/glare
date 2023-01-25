#include "kinematic_resolution_config.hpp"

#include "components/collision_component.hpp"

#include <math/math.hpp>
#include <util/variant.hpp>

namespace engine
{
	float KinematicResolutionConfig::get_size(const CollisionComponent& collision) const
	{
		float obj_size = 0.0f;
					
		util::visit
		(
			size.value,

			[&](const AABBType& aabb)
			{
				obj_size = collision.get_aabb_size();
			},

			[&](const AABBVectorType& aabb)
			{
				obj_size = collision.get_aabb_size();
			},

			[&](const OrientedAABBVectorType& aabb)
			{
				obj_size = collision.get_aabb_size();
			},

			[&](const SphereType& sphere)
			{
				obj_size = (2.0f * collision.get_bounding_radius()); // Diameter.
			},

			[&](const InnerSphereType& inner_sphere)
			{
				obj_size = collision.get_inner_diameter();
			},

			[&](const SizeType& manual_size)
			{
				//obj_size = manual_size.get_size();
				obj_size = manual_size.get_size();
			},

			[&](const VectorSizeType& vector_size)
			{
				obj_size = glm::length(vector_size.get_size());
			},

			[&](const OrientedVectorSizeType& vector_size)
			{
				obj_size = glm::length(vector_size.get_size());
			},

			[](const std::monostate&) {}
		);

		return obj_size;
	}

	float KinematicResolutionConfig::get_half_size(const CollisionComponent& collision) const
	{
		float half_obj_size = 0.0f;
					
		util::visit
		(
			size.value,

			[&](const AABBType& aabb)
			{
				half_obj_size = (collision.get_aabb_size() * 0.5f);
			},

			[&](const AABBVectorType& aabb)
			{
				half_obj_size = (collision.get_aabb_size() * 0.5f);
			},

			[&](const OrientedAABBVectorType& aabb)
			{
				half_obj_size = (collision.get_aabb_size() * 0.5f);
			},

			[&](const SphereType& sphere)
			{
				half_obj_size = collision.get_bounding_radius();
			},

			[&](const InnerSphereType& inner_sphere)
			{
				half_obj_size = collision.get_inner_radius();
			},

			[&](const SizeType& manual_size)
			{
				//obj_size = manual_size.get_size();
				half_obj_size = manual_size.get_half_size();
			},

			[&](const VectorSizeType& vector_size)
			{
				half_obj_size = glm::length(vector_size.get_half_size());
			},

			[&](const OrientedVectorSizeType& vector_size)
			{
				half_obj_size = glm::length(vector_size.get_half_size());
			},

			[](const std::monostate&) {}
		);

		return half_obj_size;
	}

	math::Vector KinematicResolutionConfig::get_size_vector(const CollisionComponent& collision) const
	{
		math::Vector obj_size = {};
					
		util::visit
		(
			size.value,

			[&](const AABBType& aabb)
			{
				auto aabb_size = collision.get_aabb_size();
				obj_size = { aabb_size, aabb_size, aabb_size };
			},

			[&](const AABBVectorType& aabb)
			{
				obj_size = collision.get_aabb_lengths();
			},

			[&](const OrientedAABBVectorType& aabb)
			{
				auto matrix = collision.get_collision_object_orientation();
				auto lengths = collision.get_aabb_lengths();

				obj_size = math::abs(matrix * lengths);
			},

			[&](const SphereType& sphere)
			{
				auto diameter = (2.0f * collision.get_bounding_radius());

				obj_size = { diameter, diameter, diameter };
			},

			[&](const InnerSphereType& inner_sphere)
			{
				auto diameter = collision.get_inner_diameter();

				obj_size = { diameter, diameter, diameter };
			},

			[&](const SizeType& manual_size)
			{
				auto length = manual_size.get_size();
				obj_size = { length, length, length };
			},

			[&](const VectorSizeType& vector_size)
			{
				obj_size = vector_size.get_size();
			},

			[&](const OrientedVectorSizeType& vector_size)
			{
				auto matrix = collision.get_collision_object_orientation();
				auto custom_size = vector_size.get_size();

				obj_size = math::abs(matrix * custom_size);
			},

			[](const std::monostate&) {}
		);

		return obj_size;
	}

	math::Vector KinematicResolutionConfig::get_half_size_vector(const CollisionComponent& collision) const
	{
		math::Vector half_obj_size = {};
					
		util::visit
		(
			size.value,

			[&](const AABBType& aabb)
			{
				auto aabb_size = (collision.get_aabb_size() * 0.5f);
				half_obj_size = { aabb_size, aabb_size, aabb_size };
			},

			[&](const AABBVectorType& aabb)
			{
				half_obj_size = (collision.get_aabb_lengths() * 0.5f);
			},

			[&](const OrientedAABBVectorType& aabb)
			{
				auto matrix = collision.get_collision_object_orientation();
				auto lengths = (collision.get_aabb_lengths() * 0.5f);

				half_obj_size = math::abs(matrix * lengths);
			},

			[&](const SphereType& sphere)
			{
				auto radius = collision.get_bounding_radius();

				half_obj_size = { radius, radius, radius };
			},

			[&](const InnerSphereType& inner_sphere)
			{
				auto radius = collision.get_inner_radius();

				half_obj_size = { radius, radius, radius };
			},

			[&](const SizeType& manual_size)
			{
				auto half_length = manual_size.get_half_size();
				half_obj_size = { half_length, half_length, half_length };
			},

			[&](const VectorSizeType& vector_size)
			{
				half_obj_size = vector_size.get_half_size();
			},

			[&](const OrientedVectorSizeType& vector_size)
			{
				auto matrix = collision.get_collision_object_orientation();
				auto half_custom_size = vector_size.get_half_size();

				half_obj_size = math::abs(matrix * half_custom_size);
			},

			[](const std::monostate&) {}
		);

		return half_obj_size;
	}
}