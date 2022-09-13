#include "kinematic_resolution_config.hpp"
#include "collision_component.hpp"

#include <util/variant.hpp>

namespace engine
{
	float KinematicResolutionConfig::get_size(const CollisionComponent& collision) const
	{
		float obj_size = 0.0f;
					
		util::visit
		(
			size,

			[&](const KinematicResolutionConfig::AABBType& aabb)
			{
				obj_size = collision.get_aabb_size();
			},

			[&](const KinematicResolutionConfig::SphereType& sphere)
			{
				obj_size = (2.0f * collision.get_bounding_radius()); // Diameter.
			},

			[&](const KinematicResolutionConfig::SizeType& manual_size)
			{
				//obj_size = manual_size.get_size();
				obj_size = manual_size.get_size();
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
			size,

			[&](const KinematicResolutionConfig::AABBType& aabb)
			{
				half_obj_size = (collision.get_aabb_size() * 0.5f);
			},

			[&](const KinematicResolutionConfig::SphereType& sphere)
			{
				half_obj_size = collision.get_bounding_radius();
			},

			[&](const KinematicResolutionConfig::SizeType& manual_size)
			{
				//obj_size = manual_size.get_size();
				half_obj_size = manual_size.get_half_size();
			},

			[](const std::monostate&) {}
		);

		return half_obj_size;
	}
}